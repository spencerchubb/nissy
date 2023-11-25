import threading
from flask import Flask, render_template, request
from scramble import get_scramble
from solve import solve

app = Flask(__name__)

def time_limit(fn, seconds):
    """Runs a function with a timeout in seconds.
    If the function does not finish in time, then return None.
    """
    result = None

    def target():
        nonlocal result
        result = fn()

    thread = threading.Thread(target=target)
    thread.start()
    thread.join(timeout=seconds)

    if thread.is_alive():
        thread.join()  # Wait for the thread to finish if it's still running
        return None
    else:
        return result

@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')

@app.route('/scramble', methods=['POST'])
def _scramble():
    body = request.get_json()
    scramble = get_scramble(body['scrambleType'])
    return { 'scramble': scramble }

@app.route('/solve', methods=['POST'])
def _solve():
    body = request.get_json()
    solutions = time_limit(
        lambda: solve(body, display_len=True),
        seconds=1
    )
    if solutions is None:
        solutions = ['Timed out after 1 second. Try using fewer steps or fewer solutions.']
    return solutions

if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=80)
