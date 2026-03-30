from PIL import Image
from pathlib import Path
from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer
import threading

ROOT_DIR = Path(__file__).parent

wd_path = ROOT_DIR / 'input'
processed_big = ROOT_DIR / 'output' / 'large' 
processed_med = ROOT_DIR / 'output' / 'small'
processed_sml = ROOT_DIR / 'output' / 'thumb'

img_queue = []

SIZES = {
        "big": 2000,
        "med": 1200,
        "sml": 400
        }

QUALITY = {
        "big": 94,
        "med": 90,
        "sml": 85
        }

class TheHandler(FileSystemEventHandler):
    def on_created(self, event):
        if not event.is_directory:
            print("Found a file")
            compiler(event.src_path)

def orchestrator():
    print("Timer Finished")
    for img in img_queue:
        converter(img)

class TheTimer:
    def __init__(self, wait_seconds=10, callback=orchestrator):
        self.wait_seconds = wait_seconds
        self.callback = callback
        self.timer = None

    def refresh(self):
        if self.timer:
            self.timer.cancel()

        self.timer = threading.Timer(self.wait_seconds, self.callback)
        self.timer.start()

timer = TheTimer()

def compiler(img_path): 
    ext = Path(img_path).suffix
    if ext.lower() != ".png" and ext.lower() != ".jpg":
        print("Wrong file type!")
    else:
        img_queue.append(img_path)
    print("timer refreshed")
    timer.refresh()

def converter(img_path):
    with Image.open(img_path) as raw_img:

        img_name_p = Path(img_path).name
        index = img_name_p.find(".")
        if index != -1:
            img_name = img_name_p[:index]
        else:
            img_name = img_name_p

        img_big = raw_img.copy()
        img_big.thumbnail((SIZES['big'], SIZES['big']), Image.Resampling.LANCZOS)
        img_big.save(f"{processed_big}/{img_name}_big.jpg", quality=QUALITY['big'])
        
        img_medium = img_big.copy()
        img_medium.thumbnail((SIZES['med'], SIZES['med']), Image.Resampling.LANCZOS)
        img_medium.save(f"{processed_med}/{img_name}_medium.jpg", quality=QUALITY['med'])

        img_small = img_medium.copy()
        img_small.thumbnail((SIZES['sml'], SIZES['sml']), Image.Resampling.LANCZOS)
        img_small.save(f"{processed_sml}/{img_name}_small.jpg", quality=QUALITY['sml'])
        print(f"Finished with {img_name}")

if __name__ == "__main__": #Set up Mock() and pass to the function
    event_handler = TheHandler()
    observer = Observer()
    observer.schedule(event_handler, str(wd_path), recursive=True)
    observer.start()
    try:
        while observer.is_alive():
            observer.join(1)
            print("H")
    finally:
        observer.stop()
        observer.join()
