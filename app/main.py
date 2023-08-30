from flask import Flask, render_template_string, request
from index import index_html
from solve import solve

app = Flask(__name__)

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        step_name = request.form.get('step_name')
        scramble = request.form.get('scramble')
        min_moves = int(request.form.get('min_moves'))
        max_moves = int(request.form.get('max_moves'))
        max_solutions = int(request.form.get('max_solutions'))
        can_niss = bool(request.form.get('can_niss'))

        solutions = solve(step_name, scramble, min_moves, max_moves, max_solutions, can_niss)
        
        return render_template_string(index_html, solutions=solutions)

    # Ideally we would use render_template instead of render_template_string.
    # However, I couldn't get it to work with pyinstaller to create an executable.
    # If we ever figure out how to make that work, we can switch to render_template.
    return render_template_string(index_html)

if __name__ == '__main__':
    app.run(debug=True)
