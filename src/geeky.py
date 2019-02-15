import sys
import argparse

import MySQLdb as mdb

def parseArgs():
    parser = argparse.ArgumentParser(description='Search for a comic based on keywords.')
    parser.add_argument('keywords', metavar='keyword', nargs='+', help='Keyword to look for')
    args = parser.parse_args()
    return args

def main():
    args = parseArgs()
    kw = args.keywords

    try:
        con = mdb.connect('localhost', 'dorian', 'moijemon', 'geeky')
        cursor = con.cursor()
        cursor.execute('select * from comics')
        rows = cursor.fetchall()

        for row in rows:
            if kw[0] in row[2]:
                print(row)

    except Exception as e:
       print(e, file=sys.stderr)


    return args


if __name__ == '__main__':
    main()
