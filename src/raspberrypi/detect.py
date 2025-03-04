# source ~/repos/mediapipe-samples/.venv/bin/activate
# Copyright 2023 The MediaPipe Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Main scripts to run hand landmarker."""

import argparse
import sys
import time
import numpy as np

import cv2
import mediapipe as mp
from picamera2 import Picamera2

from mediapipe.tasks import python
from mediapipe.tasks.python import vision
from mediapipe.framework.formats import landmark_pb2
import time
import pyautogui

mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles

# Global variables to calculate FPS
COUNTER, FPS = 0, 0
START_TIME = time.time()
DETECTION_RESULT = None

# Global detection task suspension flag
DETECT_SUSPEND = 0
# Click state machine flag
CLICK_PREV = False

def run(model: str, num_hands: int,
        min_hand_detection_confidence: float,
        min_hand_presence_confidence: float, min_tracking_confidence: float,
        camera_id: int, width: int, height: int) -> None:
    """Continuously run inference on images acquired from the camera.

  Args:
      model: Name of the hand landmarker model bundle.
      num_hands: Max number of hands that can be detected by the landmarker.
      min_hand_detection_confidence: The minimum confidence score for hand
        detection to be considered successful.
      min_hand_presence_confidence: The minimum confidence score of hand
        presence score in the hand landmark detection.
      min_tracking_confidence: The minimum confidence score for the hand
        tracking to be considered successful.
      camera_id: The camera id to be passed to OpenCV.
      width: The width of the frame captured from the camera.
      height: The height of the frame captured from the camera.
  """
    
    # Grab images as numpy arrays and leave everything else to OpenCV.
    picam2 = Picamera2()
    picam2.configure(picam2.create_preview_configuration(main={"format": 'XRGB8888', "size": (width, height)}))
    picam2.start()

    # Visualization parameters
    row_size = 50  # pixels
    left_margin = 24  # pixels
    text_color = (0, 0, 0)  # black
    font_size = 1
    font_thickness = 1
    fps_avg_frame_count = 10

    global CLICK_PREV

    # Task to manage the Mediapipe livestream detector thread
    # Limits detector queue size to 1
    def task_detect(image):
        global DETECT_SUSPEND
        if not DETECT_SUSPEND:
            # Convert the image from BGR to RGB as required by the TFLite model.
            rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_image)
            # Run hand landmarker using the model.
            detector.detect_async(mp_image, time.time_ns() // 1_000_000)
            # Suspend task_detect
            DETECT_SUSPEND = 1

    # Callback function for Mediapipe livestream detector
    def save_result(result: vision.HandLandmarkerResult,
                    unused_output_image: mp.Image, timestamp_ms: int):
        global FPS, COUNTER, START_TIME, DETECTION_RESULT, DETECT_SUSPEND

        # Calculate the FPS
        if COUNTER % fps_avg_frame_count == 0:
            FPS = fps_avg_frame_count / (time.time() - START_TIME)
            START_TIME = time.time()

        DETECTION_RESULT = result
        COUNTER += 1

        # Resume task_detect
        DETECT_SUSPEND = 0

    # Initialize the hand landmarker model
    base_options = python.BaseOptions(model_asset_path=model)
    options = vision.HandLandmarkerOptions(
        base_options=base_options,
        running_mode=vision.RunningMode.LIVE_STREAM,
        num_hands=num_hands,
        min_hand_detection_confidence=min_hand_detection_confidence,
        min_hand_presence_confidence=min_hand_presence_confidence,
        min_tracking_confidence=min_tracking_confidence,
        result_callback=save_result)
    detector = vision.HandLandmarker.create_from_options(options)

    # Continuously capture images from the camera and run inference
    while True:
        # Remove the alpha channel if present
        image = picam2.capture_array()[:, :, :3].astype(np.uint8) 
        
        # Give new frame to detection task
        task_detect(image)
        
        # Show the FPS
        fps_text = 'FPS = {:.1f}'.format(FPS)
        text_location = (left_margin, row_size)
        current_frame = image
        cv2.putText(current_frame, fps_text, text_location,
                    cv2.FONT_HERSHEY_DUPLEX,
                    font_size, text_color, font_thickness, cv2.LINE_AA)

        # Landmark visualization parameters.
        MARGIN = 10  # pixels
        FONT_SIZE = 1
        FONT_THICKNESS = 1
        HANDEDNESS_TEXT_COLOR = (88, 205, 54)  # vibrant green

        if DETECTION_RESULT:
            # Draw landmarks and indicate handedness.
            for idx in range(len(DETECTION_RESULT.hand_landmarks)):
                hand_landmarks = DETECTION_RESULT.hand_landmarks[idx]
                handedness = DETECTION_RESULT.handedness[idx]
                    
                # Get the dimensions of the current frame
                height, width, _ = current_frame.shape

                thumb_landmark = hand_landmarks[4]
                thumb_x = int(thumb_landmark.x * width)
                thumb_y = int(thumb_landmark.y * height)

                landmark = hand_landmarks[8]
                pointer_x = int(landmark.x * width)
                pointer_y = int(landmark.y * height)

                mouse_x = (thumb_x+pointer_x)/2
                mouse_y = (thumb_y+pointer_y)/2
                pyautogui.moveTo(mouse_x, mouse_y)

                distance = np.sqrt((thumb_x-pointer_x)**2+(thumb_y-pointer_y)**2)
                # print(f"Distance: {distance} px")
                click = distance < 70
                if click and not CLICK_PREV:
                    print("CLICK")
                    pyautogui.click()
                else:
                    print("NOT CLICK")
                CLICK_PREV = click

                # Draw a green dot at cursor
                cv2.circle(current_frame, (pointer_x, pointer_y), 8, (0, 255, 0), -1)  # Green filled circle
                # Draw a red dot at thumb
                cv2.circle(current_frame, (thumb_x, thumb_y), 8, (255, 0, 0), -1)  # Green filled circle

        # cv2.imshow('hand_landmarker', current_frame)
        cv2.namedWindow('hand_landmarker', cv2.WND_PROP_FULLSCREEN)
        cv2.setWindowProperty('hand_landmarker', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
        cv2.imshow('hand_landmarker', current_frame)

        # Stop the program if the ESC key is pressed.
        if cv2.waitKey(1) == 27:
            break

    detector.close()
    cv2.destroyAllWindows()


def main():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        '--model',
        help='Name of the hand landmarker model bundle.',
        required=False,
        type=str,
        default='hand_landmarker.task')
    parser.add_argument(
        '--numHands',
        help='Max number of hands that can be detected by the landmarker.',
        required=False,
        type=int,
        default=1)
    parser.add_argument(
        '--minHandDetectionConfidence',
        help='The minimum confidence score for hand detection to be considered '
             'successful.',
        required=False,
        type=float,
        default=0.5)
    parser.add_argument(
        '--minHandPresenceConfidence',
        help='The minimum confidence score of hand presence score in the hand '
             'landmark detection.',
        required=False,
        type=float,
        default=0.5)
    parser.add_argument(
        '--minTrackingConfidence',
        help='The minimum confidence score for the hand tracking to be '
             'considered successful.',
        required=False,
        type=float,
        default=0.5)
    # Finding the camera ID can be very reliant on platform-dependent methods.
    # One common approach is to use the fact that camera IDs are usually indexed sequentially by the OS, starting from 0.
    # Here, we use OpenCV and create a VideoCapture object for each potential ID with 'cap = cv2.VideoCapture(i)'.
    # If 'cap' is None or not 'cap.isOpened()', it indicates the camera ID is not available.
    parser.add_argument(
        '--cameraId',
        help='Id of camera.',
        required=False,
        type=int,
        default=0)
    parser.add_argument(
        '--frameWidth',
        help='Width of frame to capture from camera.',
        required=False,
        type=int,
        default=1280)
    parser.add_argument(
        '--frameHeight',
        help='Height of frame to capture from camera.',
        required=False,
        type=int,
        default=720)
    args = parser.parse_args()

    run(args.model, args.numHands, args.minHandDetectionConfidence,
        args.minHandPresenceConfidence, args.minTrackingConfidence,
        args.cameraId, args.frameWidth, args.frameHeight)


if __name__ == '__main__':
    main()
