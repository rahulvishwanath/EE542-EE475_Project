# EE542-EE475_Project

## Raspberry Pi Setup  

To set up the environment on your Raspberry Pi, follow these steps:  

### 1. Create a Python Virtual Environment  
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

### 2. Set Up Display Output  

Make sure an HDMI cable is connected to a monitor before running the program. Then, set the `DISPLAY` environment variable with the following command:  

```bash
export DISPLAY=:0
```

This command tells the system to send graphical output to the primary display (```:0```).

### 3. Run the Application  

Once the setup is complete, you can run the application using:  

```bash
python3 application.py
```

This command executes ```application.py```, starting the program. Ensure you are inside the ```src/raspberrypi``` directory and that the virtual environment is activated before running this command.