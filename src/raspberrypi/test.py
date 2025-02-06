#!/usr/bin/python3
import cv2

from picamera2 import Picamera2


def overlay_file_structure(frame, files):
    # Example to place text on the upper left corner of the frame
    y0, dy = 50, 30
    for i, file in enumerate(files):
        y = y0 + i*dy
        cv2.putText(frame, f'{file}', (10, y), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 1)
    return frame


# Grab images as numpy arrays and leave everything else to OpenCV.
picam2 = Picamera2()
picam2.configure(picam2.create_preview_configuration(main={"format": 'XRGB8888', "size": (640, 480)}))
picam2.start()

while True:
    im = picam2.capture_array()
    
    # Mock-up file structure
    files = ['Folder1', 'Folder2', 'Image.png', 'Video.mp4']
    im = overlay_file_structure(im, files)
    cv2.imshow("Camera", im)
    # Break the loop with 'q'
    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
picam2.stop()
cv2.destroyAllWindows()


