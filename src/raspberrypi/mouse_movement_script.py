#! python3
import pyautogui, sys
print('Press Ctrl-C to quit.')
try:
    x, y = pyautogui.position()
    positionStr = 'X: ' + str(x).rjust(4) + ' Y: ' + str(y).rjust(4)
    print(positionStr, end='')
    print('\b' * len(positionStr), end='', flush=True)
    for i in range (x, x + 800, 10):
        pyautogui.moveTo(x + i, y, 0)
    for j in range (y, y + 800, 10):
        pyautogui.moveTo(x + 800, j, 0)
except KeyboardInterrupt:
    print('\n')
