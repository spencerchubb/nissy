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

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        # If the form has a hidden input with name="solve", then we are solving.
        # Otherwise, we are getting a scramble.
        if request.form.get('solve') != None:
            step = request.form.get('step')
            scramble = request.form.get('scramble')
            num_eos = int(request.form.get('num_eos'))
            num_drs = int(request.form.get('num_drs'))
            num_htrs = int(request.form.get('num_htrs'))
            num_finishes = int(request.form.get('num_finishes'))
            nisstype = int(request.form.get('nisstype'))

            solutions = time_limit(
                lambda: solve(step, scramble, num_eos, num_drs, num_htrs, num_finishes, nisstype, display_len=True),
                seconds=1
            )
            if solutions:
                return render_template('index.html', solutions=solutions)
            else:
                return render_template('index.html', error='Timed out after 1 second. Try setting a different step or a smaller number of solutions.')
        elif request.form.get('get_scramble') != None:
            scramble_type = request.form.get('scramble_type')
            scramble = get_scramble(scramble_type)
            return render_template('index.html', scramble=scramble)

    return render_template('index.html')

if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=80)
