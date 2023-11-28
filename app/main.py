from flask import Flask, jsonify, render_template, request
from scramble import get_scramble
from solve import solve

app = Flask(__name__)

@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')

@app.route('/nissy_methods', methods=['POST'])
def nissy_methods():
    body = request.get_json()
    if body['method'] == 'scramble':
        scramble = get_scramble(body['scrambleType'])
        return { 'scramble': scramble }
    elif body['method'] == 'solve':
        return solve(body)
    
@app.errorhandler(Exception)
def handle_generic_error(e):
    response = jsonify({"error": "Internal server error."})
    response.status_code = 500
    return response

if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=80)
