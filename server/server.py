# A simple flask server for logging the pillbox event history
import datetime

from flask import Flask, render_template


app = Flask(__name__)

def log_event(event, log_file='pillbox.log'):
    time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    with open(log_file, 'a') as f:
        f.write(f'{time} {event}\n')

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/lid_open')
def lid_open():
    log_event('Lid Open')
    return 'Lid Open'

@app.route('/lid_close')
def lid_close():
    log_event('Lid Close')
    return 'Lid Close'

@app.route('/keep_alive')
def keep_alive():
    log_event('Keep Alive')
    return 'Keep Alive'

@app.route('/list_events')
def list_events():
    with open('pillbox.log', 'r') as f:
        return f.read()

if __name__ == '__main__':
    app.run()
