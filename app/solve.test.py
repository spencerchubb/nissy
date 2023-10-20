from solve import cancel_algs
import unittest

class TestCancelAlgs(unittest.TestCase):

    def test_empty(self):
        actual = cancel_algs([])
        expected = []
        self.assertEqual(actual, expected)

    def test0(self):
        actual = cancel_algs(["R U R'"])
        expected = ["R U R'"]
        self.assertEqual(actual, expected)

    def test1(self):
        actual = cancel_algs(["U R2 F2 L", "L2 U F2 L2 F U"])
        expected = ["U R2 F2 L'", "U F2 L2 F U"]
        self.assertEqual(actual, expected)

    def test2(self):
        actual = cancel_algs(["U R2 F2 L2", "L2 U F2 L2 F U"])
        expected = ["U R2 F2", "U F2 L2 F U"]
        self.assertEqual(actual, expected)

    def test3(self):
        actual = cancel_algs(["U R2 F2 L2", "L2 F U F2 L2 F U"])
        expected = ["U R2 F'", "U F2 L2 F U"]
        self.assertEqual(actual, expected)

    def test4(self):
        actual = cancel_algs(["(U R2 F2 L)", "(L2 U F2 L2 F U)"])
        expected = ["(U R2 F2 L')", "(U F2 L2 F U)"]
        self.assertEqual(actual, expected)

if __name__ == '__main__':
    unittest.main()