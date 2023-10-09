from flask import Flask, render_template, request
from scramble import get_scramble
from solve import solve

app = Flask(__name__)

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        # If the form has a hidden input with name="solve", then we are solving.
        # Otherwise, we are getting a scramble.
        if request.form.get('solve') != None:
            step_name = request.form.get('step_name')
            scramble = request.form.get('scramble')
            min_moves = int(request.form.get('min_moves'))
            max_moves = int(request.form.get('max_moves'))
            max_solutions = int(request.form.get('max_solutions'))
            nisstype = int(request.form.get('nisstype'))

            solutions = solve(step_name, scramble, min_moves, max_moves, max_solutions, nisstype)
            return render_template('index.html', solutions=solutions)
        elif request.form.get('get_scramble') != None:
            scramble_type = request.form.get('scramble_type')
            scramble = get_scramble(scramble_type)
            return render_template('index.html', scramble=scramble)

    return render_template('index.html')

if __name__ == '__main__':
    app.run(debug=True)
