import sys, re
from getopt import getopt


class Issue:
    def __init__(self, num, descr, state):
        self.num = num
        self.descr = descr
        self.state = state


def load_issues(fissues):
    re_issues = re.compile('^(\+|\-) ([0-9]+) (.+)')
    issues = {}
    for line in fissues:
        captures = re_issues.findall(line)
        if len(captures) == 1:
            state = 'open' if (captures[0][0] == '+') else 'closed'
            num = int(captures[0][1])
            descr = captures[0][2]
            issues[num] = Issue(num, descr, state)
    return issues


def list_issues(issues, args):
    filter_state = ''
    filter_descr = []
    opts, optargs = getopt(args, 's:d:', ['state=', 'descr='])
    if len(optargs) > 0:
        print 'Unrecognized arguments to ls: ' + ' '.join(optargs)
        sys.exit(1)

    for o, a in opts:
        if o in ['-s', '--state']:
            filter_state = a
        if o in ['-d', '--descr']:
            filter_descr += [re.compile(a)]

    if len(filter_state) + len(filter_descr) == 0:
        filtered = issues
    else:
        filtered = {}
        for num, issue in issues.items():
            if filter_state and issue.state == filter_state:
                filtered[num] = issue
                continue
            for regex in filter_descr:
                if regex.search(issue.descr):
                    filtered[num] = issue
                    break

    print 'Issues:'
    for num, issue in filtered.items():
        state = '+' if (issue.state == 'open') else '-'
        print ' ' + state + ' ' + str(num) + ': ' + issue.descr


def do_cmd(issues, cmd, args):
    if (cmd == 'ls'):
        list_issues(issues, args)


if __name__ == "__main__":
    fissues_path = 'issues.md'

    opts, args = getopt(sys.argv[1:], 'f:', ['file='])
    for opt, arg in opts:
        if opt in ['-f', '--file']:
            fissues_path = arg

    issues = load_issues(open(fissues_path))

    cmd = 'ls'
    cmd_args = []

    if len(args) > 0:
        cmd = args[0]
        cmd_args = args[1:]

    do_cmd(issues, cmd, cmd_args)