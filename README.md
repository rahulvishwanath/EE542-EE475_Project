# EE542-EE475_Project

## Overview

This project integrates hand tracking using MediaPipe, real-time gesture recognition using TinyML on STM32, and an augmented reality interface with Pygame. The system enables gesture-based interaction, leveraging machine learning models deployed on STM32 for real-time processing while minimizing computational load on the Raspberry Pi.

## 1. Raspberry Pi Setup  

To set up the environment on your Raspberry Pi, follow these steps:  

### 1.1. Create a Python Virtual Environment  
A virtual environment allows you to manage dependencies separately from the system Python installation, preventing conflicts with other projects.  

Run the following commands in the terminal:  

```bash
cd src/raspberrypi
python3 -m venv venv  # Create a virtual environment named 'venv'
source venv/bin/activate  # Activate the virtual environment
pip install -r requirements.txt  # Install required dependencies
```

### Explanation of Commands  

- **`python3 -m venv venv`**: Creates a virtual environment named `venv` in the current directory.  
- **`source venv/bin/activate`**: Activates the virtual environment, so installed packages are isolated from the global Python environment.  
- **`pip install -r requirements.txt`**: Installs all dependencies listed in `requirements.txt`.

### 1.2. Set Up Display Output  

Make sure an HDMI cable is connected to a monitor before running the program. Then, set the `DISPLAY` environment variable with the following command:  

```bash
export DISPLAY=:0
```

This command tells the system to send graphical output to the primary display (```:0```).

### 1.3. Run the Application  

Once the setup is complete, you can run the application using:  

```bash
python3 application.py
```

This command executes ```application.py```, starting the program. Ensure you are inside the ```src/raspberrypi``` directory and that the virtual environment is activated before running this command.

## 2. Gesture Recognition on STM32 (TinyML + Edge Impulse)

The STM32 microcontroller is responsible for running a TinyML gesture recognition model trained using Edge Impulse. The model classifies hand gestures and sends action signals to the Raspberry Pi via serial communication.

## Edge Impulse Model Training and Deployment

To train the model using Edge Impulse, follow these steps:

### 1. Collect Data:

- Record motion data from the MPU-6050 for different gestures (e.g., tap, double tap, right swipe, left swipe).
- Use Edge Impulse’s data acquisition tool to upload raw data.

### 2. Train the Model:

Set up an Impulse with:

- **Window size**: 1s or 2s (based on experimentation)
- **Feature extraction**: Spectrogram or raw data
- **Classifier**: Neural network trained on collected data.

### 3. Deploy to STM32:

- Download the trained Edge Impulse model as a **Cube.MX CMSIS-PACK** library.
- Integrate it with the STM32 firmware. Follow the steps mentioned here: [Deployment](https://docs.edgeimpulse.com/docs/run-inference/using-cubeai)

### 4. Current Setup:
- We are running **STM32GuestureV3.1.0.5.pack** for the current version.
- This is trained on sample window size of 2500 milliseconds.


