from flask import Flask, jsonify, render_template, request
from scramble import get_scramble
from shell import get_shell_result
from solve import solve
import sqlite3

class DB:
    def __init__(self):
        with sqlite3.connect('app.db') as conn:
            c = conn.cursor()
            c.execute('''CREATE TABLE IF NOT EXISTS logs (
                      id INTEGER PRIMARY KEY AUTOINCREMENT,
                      timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                      data TEXT)''')

    def log(self, data):
        '''data should be str or dict'''
        data = str(data)
        with sqlite3.connect('app.db') as conn:
            c = conn.cursor()
            c.execute('INSERT INTO logs (data) VALUES (?)', (data,))

app = Flask(__name__)
db = DB()

@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')

@app.route('/nissy_methods', methods=['POST'])
def nissy_methods():
    body = request.get_json()
    db.log(body)

    if body['method'] == 'scramble':
        scramble = get_scramble(body['scrambleType'])
        return { 'scramble': scramble }
    elif body['method'] == 'shell':
        result = get_shell_result(body['command'])
        return { 'result': result }
    elif body['method'] == 'solve':
        return solve(body)
    
@app.errorhandler(Exception)
def handle_generic_error(e):
    response = jsonify({"error": "Internal server error."})
    response.status_code = 500
    return response

if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=80)
