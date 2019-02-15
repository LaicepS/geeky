import argparse
import sys

import MySQLdb as mdb

def parseArgs():
    parser = argparse.ArgumentParser(description='Add a comic based on keywords and its origin.')
    parser.add_argument('--origin', required=True,  help="Comic's source")
    parser.add_argument('--keywords',  required=True,  help="Comics's keywords")
    return parser.parse_args()

def updateDb(cursor, origin, keywords):
    cursor.execute('select * from comics where origin = %s and keywords = %s', (origin, keywords))

    if len(cursor.fetchall()) != 0:
        return False

    cursor.execute('insert into comics(origin, keywords) values("%s", "%s")', (origin, keywords))
    return True


def main():
    args = parseArgs()
    origin = args.origin
    keywords = args.keywords

    try:
        conn = mdb.connect('localhost', 'dorian', 'moijemon', 'geeky')

        if not updateDb(conn.cursor(), origin, keywords):
            print('Did not update databse: "{}" from {} already present'.format(keywords, origin), file=sys.stderr)

        conn.commit()
        return 0
    except mdb.Error or mdb.DatabaseError or mdb.DataError as e:
        print("Error: {}".format(e), file=sys.stderr)
        return 1
     except Exception as e:
        print(e)
        return 2
    finally:
        conn.close()

if __name__ == '__main__':
    main()
