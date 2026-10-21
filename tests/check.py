#!/usr/bin/env python3
"""Compare the C unlock code (unlockers.c) with the original Python scripts.

    python3 tests/check.py      (from the NewtonUnlockOS9 folder or anywhere)
"""
import importlib.util, os, random, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.dirname(HERE)
REGISTER = os.path.dirname(SRC)

# same order as gGames in games.c
GAMES = [('columns', 'unlock_code'), ('deepspacenewt', 'reg_code'), ('mahjongg', 'reg_string'),
         ('minefieldii', 'unlock_code'), ('motile', 'unlock_code'), ('newthack', 'reg_number'),
         ('newtopoly', 'reg_code'), ('scramble', 'unlock_code'), ('seahunt', 'unlock_code'),
         ('siliconcasino', None), ('solitodx', 'unlock_code'), ('supernewtris', 'unlock_code'), ('worldcon', 'unlock_code'),
         ('xwordbuddy', 'unlock_code'), ('xwordsearch', 'unlock_code'), ('yazi', 'reg_string')]


def load(folder, fn):
    if fn is None:                       # Silicon Casino: one fixed code, from its folder
        code = open(f'{REGISTER}/{folder}/serialnumber.txt').read().strip()
        return lambda text: code
    spec = importlib.util.spec_from_file_location(folder, f'{REGISTER}/{folder}/{folder}_unlock.py')
    m = importlib.util.module_from_spec(spec)
    sys.modules[folder] = m
    spec.loader.exec_module(m)
    return getattr(m, fn)


def python_result(f, folder, text):
    try:
        if folder == 'newtopoly' and len(text) < 2:   # the script's own command-line check
            return 'ERR'
        if folder == 'deepspacenewt' and not text:
            return 'ERR'
        if getattr(f, '__name__', '') == 'unlock_code':
            t = text.strip(' \t\xa0')
            if not t.isdigit() or not t.isascii() or len(t) > 9:
                return 'ERR'
            salt = getattr(sys.modules[f.__module__], 'SALT', None)
            offset = ord(salt[0]) * 100 + ord(salt[1]) if salt else 6767   # MineField: X..X
            if int(t) < offset:     # below zero after the salt: the apps reject these
                return 'ERR'
            return f(t)
        return str(f(text))
    except Exception:
        return 'ERR'


def main():
    random.seed(9)
    macroman = [c for c in bytes(range(32, 256)).decode('mac_roman') if c.isprintable() or c == '\xa0']
    names = ['John Appleseed', 'Hare', 'A', 'ab', 'Sam', 'Jo Smith', '  Spaced Name  ', 'Ms. Émilie Dü',
             'ÆØÅ Œuvre', 'Ÿves Ωmega', 'abcdefg', '1234567', 'Zoë Ñuñez']
    cases = []
    for i, (folder, fn) in enumerate(GAMES):
        f = load(folder, fn)
        if fn is None:
            inputs = ['', 'anything', '12345678']
        elif fn == 'unlock_code':
            inputs = [str(random.randint(0, 999999999)) for _ in range(400)] + \
                     ['12345678', '10000', '6767', '6766', '0', ' 12345678 ', '12a45', '', '1234567890']
        else:
            inputs = names + [''.join(random.choice(macroman) for _ in range(random.randint(0, 40)))
                              for _ in range(400)]
        for t in inputs:
            cases.append((i, t, python_result(f, folder, t)))

    with tempfile.TemporaryDirectory() as tmp:
        exe = os.path.join(tmp, 'driver')
        subprocess.run(['cc', '-std=c89', '-Wall', '-O2', '-o', exe, f'{HERE}/driver.c',
                        f'{SRC}/unlockers.c', f'{SRC}/games.c'], check=True)
        feed = ''.join(f'{i}\t{t.encode("mac_roman").hex()}\n' for i, t, _ in cases)
        got = subprocess.run([exe], input=feed, capture_output=True, text=True, check=True).stdout.split('\n')

    bad = [(GAMES[i][0], t, want, have) for (i, t, want), have in zip(cases, got) if want != have]
    for b in bad[:20]:
        print('MISMATCH', b)
    print(f'{len(cases)} cases, {len(bad)} mismatches')
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
