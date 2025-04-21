#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys 
 
def hh():
    cgitb.enable()  # Enables error reporting
    print("Content-type: text/html\n")
    print("<html>")
    print("<body>")
    print("<h1>Python POST CGI Test</h1>")
    print("<p>Form data Post received:</p>")
    print("<pre>")
    form = cgi.FieldStorage()
    for key in form.keys():
        value = form.getvalue(key)
        print(f"{key}: {value}")

    print("</pre>")
    print("</body>")
    print("</html>")


if __name__ == '__main__':

    hh()
