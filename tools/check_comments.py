import argparse
import ast
import io
import json
from pathlib import Path
import re
import subprocess
import sys
import tokenize


ROOT = Path(__file__).resolve().parents[1]
C_LIKE_SUFFIXES = {'.c', '.cc', '.cpp', '.h', '.hpp', '.hlsl', '.hlsli'}
C_LIKE_PARTS = re.compile(
    r'(?P<raw>(?:u8|u|U|L)?R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\([\s\S]*?\)(?P=delimiter)")'
    r'|(?P<number>\b[0-9][0-9A-Za-z_.\x27]*)'
    r'|(?P<string>(?:u8|u|U|L)?"(?:\\[\s\S]|[^"\\])*")'
    r'|(?P<char>(?:u8|u|U|L)?\x27(?:\\[\s\S]|[^\x27\\\n])*\x27)'
    r'|(?P<line>//(?:\\\r?\n|[^\n])*)|(?P<block>/\*[\s\S]*?(?:\*/|\Z))')
C_LIKE_TOOL_DIRECTIVE = re.compile(
    r'//\s*(?:NOLINT(?:NEXTLINE|BEGIN|END)?(?:\([^\n]*\))?(?:\s.*)?|clang-format (?:off|on)|IWYU pragma:[^\n]+)\s*')
PYTHON_TOOL_DIRECTIVE = re.compile(
    r'#\s*(?:noqa(?:\s*:\s*[A-Z0-9, ]+)?|type:\s*[^\n]+|(?:fmt|yapf|pylint|ruff):\s*[^\n]+)\s*')
LEGAL_NOTICE = re.compile(r'copyright|SPDX-License-Identifier|SPDX-FileCopyrightText', re.IGNORECASE)


def language(name):
    path = Path(name)
    if path.suffix in C_LIKE_SUFFIXES:
        return 'c-like'
    if path.suffix == '.py':
        return 'python'
    if path.name == 'CMakeLists.txt' or path.suffix == '.cmake':
        return 'cmake'
    return None


def line_of(source, offset):
    return source.count('\n', 0, offset) + 1


def c_like_comments(source):
    for match in C_LIKE_PARTS.finditer(source):
        if match.lastgroup not in {'line', 'block'}:
            continue
        text = match.group()
        at_top = not source[:match.start()].strip()
        if (at_top and LEGAL_NOTICE.search(text)) or C_LIKE_TOOL_DIRECTIVE.fullmatch(text):
            continue
        yield line_of(source, match.start()), text


def python_comments(source):
    for token in tokenize.generate_tokens(io.StringIO(source).readline):
        if token.type != tokenize.COMMENT:
            continue
        shebang = token.start == (1, 0) and token.string.startswith('#!')
        encoding = token.start[0] <= 2 and re.fullmatch(r'#.*coding[:=]\s*[-\w.]+.*', token.string)
        if shebang or encoding or PYTHON_TOOL_DIRECTIVE.fullmatch(token.string):
            continue
        yield token.start[0], token.string
    for node in ast.walk(ast.parse(source)):
        documented = (ast.Module, ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)
        if isinstance(node, documented) and ast.get_docstring(node, clean=False) is not None:
            yield node.body[0].lineno, 'docstring'


def cmake_comments(source):
    for number, line in enumerate(source.splitlines(), 1):
        quoted = False
        for index, character in enumerate(line):
            if character == '"' and (index == 0 or line[index - 1] != '\\'):
                quoted = not quoted
            elif character == '#' and not quoted:
                yield number, line[index:]
                break


SCANNERS = {'c-like': c_like_comments, 'python': python_comments, 'cmake': cmake_comments}


def tracked_files(root):
    names = subprocess.run(['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'], cwd=root,
                           check=True, capture_output=True).stdout.decode('utf-8').split('\0')
    return sorted(name for name in names if name and language(name) and (root / name).is_file())


def check(root):
    issues = []
    files = tracked_files(root)
    for name in files:
        source = (root / name).read_text(encoding='utf-8')
        for line, text in SCANNERS[language(name)](source):
            issues.append({'path': name, 'line': line, 'comment': text.strip()[:80]})
    return {'status': 'failed' if issues else 'passed', 'checked_files': len(files), 'issues': issues}


def main(argv=None):
    parser = argparse.ArgumentParser(description='Reject explanatory comments and docstrings in the sources')
    parser.add_argument('--root', type=Path, default=ROOT)
    result = check(parser.parse_args(argv).root)
    print(json.dumps(result, indent=2))
    return int(result['status'] != 'passed')


if __name__ == '__main__':
    sys.exit(main())
