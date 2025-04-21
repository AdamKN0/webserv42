#!/usr/bin/env python3

import cgi

# Print necessary headers
# HTML boilerplate
print("<html><head><title>Simple CGI Script with cgi module</title></head>")
print("<body>")
print("<h1>Simple CGI Script with cgi module</h1>")

# Create instance of FieldStorage
form = cgi.FieldStorage()

name = ""
age = ""
# Get data from fields
try :
    name = form.getvalue('name')
    age = form.getvalue('age')
except :
    print("No name nor age")


# Print HTML form
print("<form method='post' action='python.py'>")
print("Name: <input type='text' name='name'><br>")
print("Age: <input type='text' name='age'><br>")
print("<input type='submit' value='Submit'>")
print("</form>")

# Print submitted data
if name or age:
    print("<h2>Submitted Data:</h2>")
    print("<p>Name: {}</p>".format(name))
    print("<p>Age: {}</p>".format(age))

print("</body></html>")
