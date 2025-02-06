import os
import sys
import cv2
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget, QPushButton, QTreeView, QFileSystemModel, QDialog, QHBoxLayout
from PyQt5.QtCore import QTimer, Qt, QPoint, QRect
from PyQt5.QtGui import QImage, QPixmap
from picamera2 import Picamera2

class LauncherWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initCamera()
        self.initUI()

    def initCamera(self):
        self.picam2 = Picamera2()
        preview_config = self.picam2.create_preview_configuration(main={"format": 'XRGB8888', "size": (1536, 864)})
        self.picam2.configure(preview_config)
        self.picam2.start()

    def initUI(self):
        self.setWindowTitle('AR-like File Explorer Launcher')
        self.setGeometry(100, 100, 1536, 864)
        self.centralWidget = QWidget(self)
        self.setCentralWidget(self.centralWidget)
        self.layout = QVBoxLayout(self.centralWidget)

        self.videoLabel = QLabel(self)
        self.videoLabel.setGeometry(0, 0, 1536, 864)
        self.layout.addWidget(self.videoLabel)

        self.openExplorerButton = QPushButton("Open File Explorer", self)
        self.openExplorerButton.setGeometry(20, 20, 180, 40)
        self.openExplorerButton.setStyleSheet("background-color: rgba(255, 255, 255, 0.5); font-weight: bold; border-radius: 20px;")
        self.openExplorerButton.clicked.connect(self.toggleFileExplorer)

        self.fileExplorer = FileExplorerPanel(self.videoLabel)
        self.fileExplorer.hide()

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_frame)
        self.timer.start(30)

    def update_frame(self):
        frame = self.picam2.capture_array()
        if frame is not None:
            image = self.convert_cv_qt(frame)
            self.videoLabel.setPixmap(image)

    def convert_cv_qt(self, cv_img):
        rgb_image = cv2.cvtColor(cv_img, cv2.COLOR_BGR2RGB)
        h, w, ch = rgb_image.shape
        bytes_per_line = ch * w
        convert_to_Qt_format = QImage(rgb_image.data, w, h, bytes_per_line, QImage.Format_RGB888)
        return QPixmap.fromImage(convert_to_Qt_format)

    def toggleFileExplorer(self):
        if self.fileExplorer.isHidden():
            self.fileExplorer.show()
            self.fileExplorer.move((self.width() - self.fileExplorer.width()) // 2, (self.height() - self.fileExplorer.height()) // 2)
        else:
            self.fileExplorer.raise_()

class FileExplorerPanel(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent, Qt.FramelessWindowHint)
        self.setWindowOpacity(0.95)
        self.resize(900, 600)
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()
        self.model = QFileSystemModel()
        self.model.setRootPath('/home/rahul')

        self.tree = QTreeView()
        self.tree.setModel(self.model)
        self.tree.setRootIndex(self.model.index('/home/rahul'))
        self.tree.doubleClicked.connect(self.openImage)
        layout.addWidget(self.tree)

        self.navLayout = QHBoxLayout()
        self.backButton = QPushButton("Back")
        self.backButton.setStyleSheet("QPushButton {border-radius: 10px;} QPushButton:hover {background-color: rgba(255, 255, 255, 0.7);}")
        self.backButton.clicked.connect(self.navigate_back)
        self.closeButton = QPushButton("Close")
        self.closeButton.setStyleSheet("QPushButton {border-radius: 10px;} QPushButton:hover {background-color: rgba(255, 255, 255, 0.7);}")
        self.closeButton.clicked.connect(self.close)
        self.navLayout.addWidget(self.backButton)
        self.navLayout.addWidget(self.closeButton)
        layout.addLayout(self.navLayout)

        self.setLayout(layout)

    def navigate_back(self):
        currentRootIndex = self.tree.rootIndex()
        parentIndex = self.model.parent(currentRootIndex)
        if parentIndex.isValid():
            self.tree.setRootIndex(parentIndex)

    def openImage(self, index):
        filePath = self.model.filePath(index)
        if not self.model.isDir(index) and filePath.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif')):
            self.imageViewer = ImageViewer(filePath, self)
            self.imageViewer.show()

    '''def mousePressEvent(self, event):
	super().mousePressEvent(event)
        self.raise_()
        self.activateWindow()
	self.oldPos = event.globalPos()'''

    def mousePressEvent(self, event):
        super().mousePressEvent(event)  # Ensure this line is properly indented
        self.raise_()
        self.activateWindow()
        self.oldPos = event.globalPos()

    def mouseMoveEvent(self, event):
        delta = QPoint(event.globalPos() - self.oldPos)
        self.move(self.x() + delta.x(), self.y() + delta.y())
        self.oldPos = event.globalPos()

class ImageViewer(QDialog):
    def __init__(self, image_path, parent=None):
        super().__init__(parent, Qt.FramelessWindowHint)
        self.setWindowOpacity(0.9)
        self.setWindowTitle("Image Viewer")
        self.setGeometry(200, 200, 600, 400)  # Adjustable size
        self.image_path = image_path
        self.image_directory = os.path.dirname(image_path)
        self.images = self.load_image_files(self.image_directory)
        self.current_image_index = self.images.index(image_path)
        self.initUI()

    def initUI(self):
        self.layout = QVBoxLayout()
        self.imageLabel = QLabel(self)
        self.imageLabel.setAlignment(Qt.AlignCenter)

        # Navigation layout
        self.navLayout = QHBoxLayout()
        self.prevButton = QPushButton("Previous")
        self.prevButton.clicked.connect(self.prev_image)
        self.nextButton = QPushButton("Next")
        self.nextButton.clicked.connect(self.next_image)
        self.closeButton = QPushButton("Close")
        self.closeButton.clicked.connect(self.close)
        
        self.navLayout.addWidget(self.prevButton)
        self.navLayout.addWidget(self.nextButton)
        self.navLayout.addWidget(self.closeButton)
        
        self.layout.addWidget(self.imageLabel, 1)  # Make sure the image label takes most of the space
        self.layout.addLayout(self.navLayout)
        self.setLayout(self.layout)

    def load_image_files(self, directory):
        """ Load all image files from the directory containing the initially opened image. """
        supported_extensions = ['.png', '.jpg', '.jpeg', '.bmp', '.gif']
        return [os.path.join(directory, f) for f in os.listdir(directory) 
                if os.path.isfile(os.path.join(directory, f)) and f.lower().endswith(tuple(supported_extensions))]

    def update_image(self, image_path):
        """ Update the displayed image. """
        pixmap = QPixmap(image_path)
        self.imageLabel.setPixmap(pixmap.scaled(self.imageLabel.width(), self.imageLabel.height(), Qt.KeepAspectRatio, Qt.SmoothTransformation))

    def prev_image(self):
        """ Move to the previous image in the directory. """
        if self.current_image_index > 0:
            self.current_image_index -= 1
            self.update_image(self.images[self.current_image_index])

    def next_image(self):
        """ Move to the next image in the directory. """
        if self.current_image_index < len(self.images) - 1:
            self.current_image_index += 1
            self.update_image(self.images[self.current_image_index])

    def showEvent(self, event):
        """ Update the image once the widget is visible to ensure correct scaling. """
        super().showEvent(event)
        self.update_image(self.images[self.current_image_index])


    ''' def mousePressEvent(self, event):
        self.oldPos = event.globalPos()
	super().mousePressEvent(event)
        self.raise_()
        self.activateWindow()'''

    def mousePressEvent(self, event):
        super().mousePressEvent(event)  # Ensure this line is properly indented
        self.raise_()
        self.activateWindow()
        self.oldPos = event.globalPos()

    def mouseMoveEvent(self, event):
        delta = QPoint(event.globalPos() - self.oldPos)
        self.move(self.x() + delta.x(), self.y() + delta.y())
        self.oldPos = event.globalPos()

def main():
    app = QApplication(sys.argv)
    launcher = LauncherWindow()
    launcher.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
