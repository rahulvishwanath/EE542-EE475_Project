import pygame
import numpy as np
import random
from datetime import datetime
from picamera2 import Picamera2
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
from mediapipe.framework.formats import landmark_pb2
import pyautogui
import time
import cv2
import requests
import serial
import yfinance as yf
import io

# Setup pygame
pygame.init()

# Window size
WIDTH, HEIGHT = 1280, 720
screen = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)
pygame.display.set_caption("AR Game with Hand Tacking")

# Font
font = pygame.font.Font(None, 36)

# Picam setup
picam2 = Picamera2()
picam2.configure(picam2.create_preview_configuration(main={"format": 'XRGB8888', "size": (WIDTH, HEIGHT)}))
picam2.start()

# Sprite setup
SPRITE_SIZE = 120
sprite_img = pygame.image.load("turtle.png")  # Replace with the path to your logo image
sprite_img = pygame.transform.scale(sprite_img, (SPRITE_SIZE, SPRITE_SIZE))
sprite_img_org = sprite_img
sprite_img = pygame.transform.rotate(sprite_img_org, random.randint(0, 360))
sprite_pos = [random.randint(0, WIDTH - SPRITE_SIZE), random.randint(HEIGHT // 2, HEIGHT - SPRITE_SIZE)]

# Reset Button dimensions
RESET_X = 10
RESET_Y = 10
RESET_WIDTH = 100
RESET_HEIGHT = 50

# Open/Close Button dimensions
GAME_BUTTON_WIDTH = 150
GAME_BUTTON_HEIGHT = 50
GAME_BUTTON_X = WIDTH - GAME_BUTTON_WIDTH - 10
GAME_BUTTON_Y = 10

# Game variables
score = 0
clock = pygame.time.Clock()
game_active = False

# Weather API
API_KEY = '' # Add your own API key from weatherapi.com
CITY = 'Seattle'
WEATHER_URL = f'http://api.weatherapi.com/v1/current.json?key={API_KEY}&q={CITY}&aqi=no'
weather_img = None

# Stock variables
stock_ticker_x = WIDTH // 2 + 200
stock_symbols = ['NVDA', 'GOOGL', 'AMZN', 'MSFT', 'AMD']
stock_prices = []
current_stock_index = 0
last_stock_switch_time = time.time()
STOCK_SWITCH_INTERVAL = 3

# Mediapipe Hand Tracking
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles

# Global variables
DETECTION_RESULT = None
DETECT_SUSPEND = 0
CLICK_PREV = False
FPS = 0
COUNTER = 0
START_TIME = time.time()
RECT_ALPHA = 190

# Gestures and Widgets
GESTURE = ''
CURRENT_WIDGET = 'weather'
NEXT_WIDGET = None
WIDGET_TRANSITION_OUT = False
WIDGET_TRANSITION_IN = False
WIDGET_ALPHA = 255
UPDATE_INTERVAL = 60 # 60 seconds

# Initialize the hand landmarker detector
def initialize_detector(model: str, num_hands: int, min_hand_detection_confidence: float,
                        min_hand_presence_confidence: float, min_tracking_confidence: float):
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
    return detector

# Callback function for Mediapipe livestream detector
def save_result(result: vision.HandLandmarkerResult, unused_output_image: mp.Image, timestamp_ms: int):
    global FPS, COUNTER, START_TIME, DETECTION_RESULT, DETECT_SUSPEND

    if COUNTER % 10 == 0:
        FPS = 10 / (time.time() - START_TIME)
        START_TIME = time.time()

    DETECTION_RESULT = result
    COUNTER += 1
    DETECT_SUSPEND = 0

# Process the frame and detect hands
def process_frame(detector, image, width, height):
    global CLICK_PREV, DETECT_SUSPEND, ticker_speed, GESTURE # DITTO GESTURE

    if not DETECT_SUSPEND:
        # Convert the image from BGR to RGB as required by the TFLite model.
        rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_image)
        
        # Run hand landmarker using the model
        detector.detect_async(mp_image, time.time_ns() // 1_000_000)
        DETECT_SUSPEND = 1

    # Process detection result
    if DETECTION_RESULT:
        # ticker_speed = 18
        for idx in range(len(DETECTION_RESULT.hand_landmarks)):
            hand_landmarks = DETECTION_RESULT.hand_landmarks[idx]

            # Get landmarks
            thumb_landmark = hand_landmarks[4]
            pointer_landmark = hand_landmarks[8]
            pointer_knuckle_landmark = hand_landmarks[5]

            # Invert the x-coordinate to mirror the image
            thumb_x = int((1 - thumb_landmark.x) * width)
            thumb_y = int(thumb_landmark.y * height)
            pointer_x = int((1 - pointer_landmark.x) * width)
            pointer_y = int(pointer_landmark.y * height)
            mouse_x = int((1 - pointer_knuckle_landmark.x) * width)
            mouse_y = int(pointer_knuckle_landmark.y * height)
            pyautogui.moveTo(pointer_x, pointer_y)

    return image


def get_weather():
    global weather_img
    try:
        response = requests.get(WEATHER_URL)
        data = response.json()
        temperature = data['current']['temp_f'] # Temperature in F
        weather_description = data['current']['condition']['text']  # Weather description
        image = requests.get('https:' + data['current']['condition']['icon'], stream=True)

        if response.status_code == 200:
            weather_img = pygame.image.load(io.BytesIO(image.content))
        else:
            print("Failed to load image:", response.status_code)

        return f"{weather_description}, {temperature}°F"
    except Exception as e:
        return "Weather info unavailable"

def get_stock_price(ticker):
    try:
        stock = yf.Ticker(ticker)
        current_price = stock.history(period="1d")['Close'].iloc[-1]

        return f"{current_price:.2f}"
    except Exception as e:
        return "Stock Data Unavailable"
    
def fetch_stock_prices():
    global stock_text
    stock_prices.clear()

    for symbol in stock_symbols:
        price = get_stock_price(symbol)
        stock_prices.append(f"{price}")

def update_widget():
    global CURRENT_WIDGET, GESTURE
    if GESTURE in ['', 'idle']:
        return

    widget_order = ['weather', 'time', 'stock']
    curr_idx = widget_order.index(CURRENT_WIDGET)

    if GESTURE == 'leftswipe':
        start_widget_transition(widget_order[(curr_idx + 1) % len(widget_order)])
    elif GESTURE == 'rightswipe':
        start_widget_transition(widget_order[(curr_idx - 1) % len(widget_order)])
    
    GESTURE = 'idle'

def start_widget_transition(new_widget):
    global WIDGET_TRANSITION_OUT, WIDGET_ALPHA, NEXT_WIDGET
    if new_widget != CURRENT_WIDGET:
        WIDGET_TRANSITION_OUT = True
        WIDGET_ALPHA = 255
        NEXT_WIDGET = new_widget

def update_widget_transition():
    global CURRENT_WIDGET, WIDGET_TRANSITION_OUT, WIDGET_TRANSITION_IN, WIDGET_ALPHA
    if WIDGET_TRANSITION_OUT:
        WIDGET_ALPHA -= 40
        if WIDGET_ALPHA <= 0:
            CURRENT_WIDGET = NEXT_WIDGET
            WIDGET_ALPHA = 0
            WIDGET_TRANSITION_OUT = False
            WIDGET_TRANSITION_IN = True
    if WIDGET_TRANSITION_IN:
        WIDGET_ALPHA = min(255, WIDGET_ALPHA + 40)
        if WIDGET_ALPHA == 255:
            WIDGET_TRANSITION_IN = False

# Function to draw rounded rectangles (for professional UI look)
def draw_rounded_rect(size, color, location, corner_radius=10):
    panel_surface = pygame.Surface(size, pygame.SRCALPHA)
    pygame.draw.rect(panel_surface, color, panel_surface.get_rect(), border_radius=corner_radius)
    screen.blit(panel_surface, location)

def draw_widgets(weather):  
    global current_stock_index, last_stock_switch_time
    panel_width = 400
    panel_height = 80
    panel_x = (WIDTH // 2) - (panel_width // 2)
    panel_y = 10

    draw_rounded_rect((panel_width, panel_height), (32, 32, 32, RECT_ALPHA), (panel_x, panel_y), 15)

    if CURRENT_WIDGET == 'time':
        widget_text = datetime.now().strftime("%H:%M:%S")
    elif CURRENT_WIDGET == 'weather':
        widget_text = weather
    elif CURRENT_WIDGET == 'stock':
        if time.time() - last_stock_switch_time > STOCK_SWITCH_INTERVAL:
            current_stock_index = (current_stock_index + 1) % len(stock_symbols)
            last_stock_switch_time = time.time()

        stock_text = f"{stock_symbols[current_stock_index]}: ${stock_prices[current_stock_index]}"
        widget_text = stock_text
    
    text_surface = font.render(widget_text, True, (255, 255, 255))
    text_surface.set_alpha(WIDGET_ALPHA)

    text_width, text_height = font.size(widget_text)

    text_x = panel_x + (panel_width - text_width) // 2
    text_y = panel_y + (panel_height - text_height) // 2

    image_width = weather_img.get_width() if weather_img else 0
    spacing = 10  # Space between image and text

    total_width = image_width + spacing + text_width  # Combined width of image + text
    start_x = panel_x + (panel_width - total_width) // 2  # Center the whole content

    # Draw image
    if weather_img is not None and CURRENT_WIDGET == 'weather':
        screen.blit(weather_img, (start_x, panel_y + (panel_height - weather_img.get_height()) // 2))
        text_x = start_x + image_width + spacing
    else:
        text_x = panel_x + (panel_width - text_width) // 2  # Center text if no image

    text_y = panel_y + (panel_height - text_height) // 2

    screen.blit(text_surface, (text_x, text_y))

    update_widget_transition()

def draw_buttons():
    start_button = pygame.Rect(GAME_BUTTON_X, GAME_BUTTON_Y, GAME_BUTTON_WIDTH, GAME_BUTTON_HEIGHT)
    close_button = pygame.Rect(GAME_BUTTON_X, GAME_BUTTON_Y, GAME_BUTTON_WIDTH, GAME_BUTTON_HEIGHT)

    if not game_active:
        draw_rounded_rect((GAME_BUTTON_WIDTH, GAME_BUTTON_HEIGHT), (0, 255, 0, RECT_ALPHA), (GAME_BUTTON_X, GAME_BUTTON_Y), 15)
        text_width, text_height = font.size("Start Game")
        screen.blit(font.render("Start Game", True, (0, 0, 0)), (GAME_BUTTON_X + (GAME_BUTTON_WIDTH - text_width) // 2, GAME_BUTTON_Y + (GAME_BUTTON_HEIGHT - text_height) // 2))
    else:
        draw_rounded_rect((GAME_BUTTON_WIDTH, GAME_BUTTON_HEIGHT), (0, 255, 0, RECT_ALPHA), (GAME_BUTTON_X, GAME_BUTTON_Y), 15)
        text_width, text_height = font.size("Stop Game")
        screen.blit(font.render("Stop Game", True, (0, 0, 0)), (GAME_BUTTON_X + (GAME_BUTTON_WIDTH - text_width) // 2, GAME_BUTTON_Y + (GAME_BUTTON_HEIGHT - text_height) // 2))

    return start_button, close_button

# Main function to initialize and process frames
def main():
    global sprite_pos, sprite_img, score, game_active, GESTURE
    # Initialize the detector
    detector = initialize_detector(
        model='hand_landmarker.task',
        num_hands=1,
        min_hand_detection_confidence=0.5,
        min_hand_presence_confidence=0.5,
        min_tracking_confidence=0.5
    )

    ser = serial.Serial(
       port = "/dev/ttyAMA0",
       baudrate = 115200,
       parity = serial.PARITY_NONE,
       stopbits = serial.STOPBITS_ONE,
       bytesize = serial.EIGHTBITS,
       timeout = 1)

    # Fetch API data
    weather_string = get_weather()
    fetch_stock_prices()
    last_update = time.time()

    while True:
        for event in pygame.event.get():
          if event.type == pygame.MOUSEBUTTONDOWN:
                # Check if click is on sprite
                mouse_x, mouse_y = event.pos
                start_button, close_button = draw_buttons()

                if sprite_pos[0] <= mouse_x <= sprite_pos[0] + SPRITE_SIZE and sprite_pos[1] <= mouse_y <= sprite_pos[1] + SPRITE_SIZE:
                    score += 1
                    sprite_pos = [random.randint(150, WIDTH - 150 - SPRITE_SIZE), random.randint(350, HEIGHT - 150 - SPRITE_SIZE)]
                    sprite_img = pygame.transform.rotate(sprite_img_org, random.randint(0, 360))
                    sprite_img = pygame.transform.scale(sprite_img, (SPRITE_SIZE, SPRITE_SIZE))
                if RESET_X <= mouse_x <= RESET_X + RESET_WIDTH and RESET_Y <= mouse_y <= RESET_Y + RESET_HEIGHT:
                    score = 0

                if not game_active and start_button.collidepoint(mouse_x, mouse_y):
                    game_active = True  # Start game
                elif game_active and close_button.collidepoint(mouse_x, mouse_y):
                    game_active = False  # Close game
                    score = 0

        # Capture one frame
        image = picam2.capture_array()[:, :, :3]
        bgr_image = cv2.cvtColor(picam2.capture_array(), cv2.COLOR_BGR2RGB)
        bgr_image = cv2.flip(bgr_image, 1)
        image = cv2.flip(image, 1)
        
        # Process frame for hand detection
        processed_image = process_frame(detector, image, WIDTH, HEIGHT)

        # Convert frame to Pygame format
        frame = np.rot90(bgr_image)
        frame = pygame.surfarray.make_surface(frame)
        screen.blit(frame, (0, 0))

        # Update any of the API strings if needed
        if time.time() - last_update > UPDATE_INTERVAL:
            fetch_stock_prices()
            weather_string = get_weather()
            last_update = time.time()

        if ser.in_waiting > 0:
            GESTURE = ser.readline().decode().strip()
            print(f"Received: {GESTURE}")

        if GESTURE == 'doubletap':
            pyautogui.click()
            GESTURE = 'idle'

        if not game_active:
            # Update what widget is supposed to be displayed based on gesture
            update_widget()
            
            # Draw Widgets
            draw_widgets(weather_string)
        else:
            if GESTURE == 'leftswipe' or GESTURE == 'rightswipe':
                GESTURE = 'doubletap'
            # Reset Button setup
            reset_panel = pygame.Rect(RESET_X, RESET_Y, RESET_WIDTH, RESET_HEIGHT)
            # draw_rounded_rect(screen, (32, 32, 32), reset_panel, 15)
            draw_rounded_rect((RESET_WIDTH, RESET_HEIGHT), (32, 32, 32, RECT_ALPHA), (RESET_X, RESET_Y), 15)
            reset_text = font.render("Reset", True, (255, 255, 255))
            text_width, text_height = font.size("Reset")
            screen.blit(reset_text, (RESET_X + (RESET_WIDTH - text_width) // 2, RESET_Y + (RESET_HEIGHT - text_height) // 2))

            # Draw Game (Bottom Section)
            screen.blit(sprite_img, sprite_pos)
            score_text = font.render(f"Score: {score}", True, (255, 255, 255))
            screen.blit(score_text, (10, HEIGHT - 40))

        draw_buttons()
        
        pygame.display.update()
        clock.tick(30)
        
        # Exit if the ESC key is pressed
        if cv2.waitKey(1) == 27:
            break

    picam2.stop()
    cv2.destroyAllWindows()
    pygame.quit()

if __name__ == '__main__':
    main()
