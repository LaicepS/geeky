import argparse

def parseArgs():
    parser = argparse.ArgumentParser(description='Search for a comic based on keywords.')
    parser.add_argument('strings', metavar='keyword', nargs='+', help='Keyword to look for')
    args = parser.parse_args()

def main():
    parseArgs()

if __name__ == '__main__':
    main()
