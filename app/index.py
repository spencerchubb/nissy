index_html = '''
<!DOCTYPE html>
<html>

<head>
    <title>Nissy Web Interface</title>

    <style>
        body {
            max-width: 800px;
            width: 100%;
            margin: auto;
            padding-bottom: 32px;
        }

        p, li, label, button, input, textarea, select {
            font-size: 1rem;
            font-family: sans-serif;
        }

        form {
            display: grid;
            grid-template-columns: auto 1fr;
            grid-gap: 16px;
        }

        label {
            grid-column: 1 / 2;
        }

        input[type="checkbox"] {
            width: 1rem;
            height: 1rem;
            margin: 0;
        }

        button[type="submit"] {
            grid-column: 1 / 3;
            background-color: #007bff;
            color: white;
            border: none;
            cursor: pointer;
            padding: 8px 16px;
            border-radius: 4px;
        }

        button[type="submit"]:hover {
            background-color: #0056b3;
        }

        .divider {
            border-top: dashed 3px #bbb;
            width: 100%;
            margin: 48px 0;
        }
    </style>

    <script>
        let steps = {
            "optimal": "Optimal solve (in HTM)",
            "light": "Optimal solve (in HTM), small table (500Mb RAM total)",
            "eofin": "Optimal solve after EO without breaking EO (detected)",
            "eofbfin": "Optimal after EO on F/B without breaking EO",
            "eorlfin": "Optimal after EO on R/L without breaking EO",
            "eoudfin": "Optimal after EO on U/D without breaking EO",
            "eo": "EO on any axis",
            "eofb": "EO on F/B",
            "eorl": "EO on R/L",
            "eoud": "EO on U/D",
            "co": "CO on any axis",
            "coud": "CO on U/D",
            "corl": "CO on R/L",
            "cofb": "CO on F/B",
            "co-URF": "CO any axis (URF moveset)",
            "coud-URF": "CO on U/D (URF moveset)",
            "corl-URF": "CO on R/L (URF moveset)",
            "cofb-URF": "CO on F/B (URF moveset)",
            "dr": "DR on any axis",
            "drud": "DR on U/D",
            "drrl": "DR on R/L",
            "drfb": "DR on F/B",
            "dr-eo": "DR without breaking EO (automatically detected)",
            "dr-eofb": "DR on U/D or R/L without breaking EO on F/B",
            "dr-eorl": "DR on U/D or F/B without breaking EO on R/L",
            "dr-eoud": "DR on R/L or F/B without breaking EO on U/D",
            "drud-eofb": "DR on U/D without breaking EO on F/B",
            "drrl-eofb": "DR on R/L without breaking EO on F/B",
            "drud-eorl": "DR on U/D without breaking EO on R/L",
            "drfb-eorl": "DR on F/B without breaking EO on R/L",
            "drfb-eoud": "DR on F/B without breaking EO on U/D",
            "drrl-eoud": "DR on R/L without breaking EO on U/D",
            "drfin": "DR finish on any axis without breaking DR",
            "drudfin": "DR finish on U/D without breaking DR",
            "drrlfin": "DR finish on R/L without breaking DR",
            "drfbfin": "DR finish on F/B without breaking DR",
            "htr": "HTR from DR",
            "htr-drud": "HTR from DR on U/D",
            "htr-drrl": "HTR from DR on R/L",
            "htr-drfb": "HTR from DR on F/B",
            "corners-dr": "Solve corners from DR",
            "corners-drud": "Solve corners from DR on U/D",
            "corners-drrl": "Solve corners from DR on R/L",
            "corners-drfb": "Solve corners from DR on F/B",
            "htrfin": "HTR finish without breaking HTR",
            "chtr": "Solve corners to HTR state",
            "chtr-URF": "Solve corners to HTR state (URF moveset)",
            "corners": "Solve corners",
            "corners-URF": "Solve corners (URF moveset)"
        };

        let scrambleTypes = [
            "normal",
            "eo",
            "dr",
            "htr",
            "corners",
            "edges",
            "fmc",
        ];

        function loadFormValues() {
            const stepSelect = document.querySelector('[name="step_name"]');
            const scrambleTypeSelect = document.querySelector('[name="scramble_type"]');

            stepSelect.value = localStorage.getItem('step_name') || '';
            document.querySelector('[name="scramble"]').value = localStorage.getItem('scramble') || '';
            document.querySelector('[name="min_moves"]').value = localStorage.getItem('min_moves') || 1;
            document.querySelector('[name="max_moves"]').value = localStorage.getItem('max_moves') || 20;
            document.querySelector('[name="max_solutions"]').value = localStorage.getItem('max_solutions') || 1;
            document.querySelector('[name="can_niss"]').checked = localStorage.getItem('can_niss') === 'true';
            scrambleTypeSelect.value = localStorage.getItem('scramble_type') || '';

            // Load and populate step_name select options
            for (const stepKey in steps) {
                const option = document.createElement('option');
                option.value = stepKey;
                option.text = steps[stepKey];
                stepSelect.appendChild(option);
            }
            const savedStep = localStorage.getItem('step_name');
            if (savedStep) {
                stepSelect.value = savedStep;
            }

            // Load and populate scramble_type select options
            for (const scrambleType of scrambleTypes) {
                const option = document.createElement('option');
                option.value = scrambleType;
                option.text = scrambleType;
                scrambleTypeSelect.appendChild(option);
            }
            const savedScrambleType = localStorage.getItem('scramble_type');
            if (savedScrambleType) {
                scrambleTypeSelect.value = savedScrambleType;
            }
        }

        // Function to save form values to local storage
        function saveFormValues() {
            localStorage.setItem('step_name', document.querySelector('[name="step_name"]').value);
            localStorage.setItem('scramble', document.querySelector('[name="scramble"]').value);
            localStorage.setItem('min_moves', document.querySelector('[name="min_moves"]').value);
            localStorage.setItem('max_moves', document.querySelector('[name="max_moves"]').value);
            localStorage.setItem('max_solutions', document.querySelector('[name="max_solutions"]').value);
            localStorage.setItem('can_niss', document.querySelector('[name="can_niss"]').checked);
            localStorage.setItem('scramble_type', document.querySelector('[name="scramble_type"]').value);
        }
    </script>
</head>

<body onload="loadFormValues()">
    <h1>Nissy Web Interface</h1>
    <p>
        This is an easy web interface for the nissy tool pioneered by Sebastiano Tronto.
        Nissy is an FMC tool that helps you find EOs, DRs, and HTRs.
    </p>
    <p>
        See <a href="https://github.com/spencerchubb/nissy">the github repo</a> for the code and technical details.
    </p>

    <form method="post" onchange="saveFormValues()">
        <input type="hidden" name="solve">

        <label for="step_name">Step Name:</label>
        <select name="step_name" required></select>

        <label for="scramble">Scramble:</label>
        <textarea type="text" name="scramble" required></textarea>

        <label for="min_moves">Min Moves:</label>
        <input type="number" name="min_moves" min="1" max="20" required>

        <label for="max_moves">Max Moves:</label>
        <input type="number" name="max_moves" min="1" max="20" required>

        <label for="max_solutions">Max Solutions:</label>
        <input type="number" name="max_solutions" min="1" required>

        <label for="can_niss">Can Niss:</label>
        <input type="checkbox" name="can_niss">

        <button type="submit">Solve</button>
    </form>

    {% if solutions %}
    <h2>Solutions:</h2>
    <ul>
        {% for solution in solutions %}
        <li>{{ solution }}</li>
        {% endfor %}
    </ul>
    {% endif %}

    <div class="divider"></div>

    <form method="post" onchange="saveFormValues()">
        <input type="hidden" name="get_scramble">

        <label for="scramble_type">Scramble Type:</label>
        <select name="scramble_type" required></select>

        <button type="submit">Scramble</button>
    </form>

    {% if scramble %}
    <h2>Scramble:</h2>
    <p>{{ scramble }}</p>
    {% endif %}
</body>

</html>
'''