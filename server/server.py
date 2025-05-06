#!/usr/bin/env python

# A simple flask server for logging the pillbox event history
import datetime

from flask import Flask, render_template


app = Flask(__name__)

def log_event(event, log_file='pillbox.log'):
    time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print('%s %s' % (time, event))
    with open(log_file, 'a') as f:
        f.write(f'{time} {event} <br>\n')

@app.route('/')
def index():
    return "<h1>Pillbox Server</h1><h3>Available commands</h3>list_events_all, list_events, lid_open, lid_close, keep_alive"

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


# Debugging route with additional url encoded "txt" parameter to log
@app.route('/debug/<txt>')
def log(txt):
    log_event("DEBUG: %s" % txt)
    return txt


@app.route('/list_events_all')
def list_events_all():
    with open('pillbox.log', 'r') as f:
        return f.read()

@app.route('/list_events')
def list_events():
    output=''
    with open('pillbox.log', 'r') as f:
        for cline in f:
            if 'Keep Alive' not in cline:
                output+=cline
    return output

if __name__ == '__main__':
    app.run(host="0.0.0.0")
