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
    return list_events_reverse()
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
    log_event('keep_alive')
    return 'Keep Alive'


# Debugging route with additional url encoded "txt" parameter to log
@app.route('/debug/<txt>')
def log(txt):
    log_event("DEBUG: %s" % txt)
    return txt


@app.route('/list_events_all')
def list_events_all():
    with open('pillbox.log', 'r') as f:
        # Read the entire log file and return it as a string but with lines in backwards order (last line first)
        return f.read()

@app.route('/list_events')
def list_events():
    output=''
    with open('pillbox.log', 'r') as f:
        for cline in f:
            if 'keep_alive' not in cline:
                output+=cline
    # change output to be in reverse order (last line first)
    return output

@app.route('/list_events_all_reverse')
def list_events_all_reverse():
    with open('pillbox.log', 'r') as f:
        # Read the entire log file and return it as a string but with lines in backwards order (last line first)
        lines = f.readlines()
        lines.reverse()
        return ''.join(lines)
        # return f.read()

@app.route('/list_events_reverse')
def list_events_reverse():
    output=''
    with open('pillbox.log', 'r') as f:
        for cline in f:
            if 'keep_alive' not in cline:
                output+=cline
    # change output to be in reverse order (last line first)
    output = output.splitlines()[::-1]
    output = '<br>\n'.join(output)
    return output


if __name__ == '__main__':
    app.run(host="0.0.0.0")
