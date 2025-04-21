#!/usr/bin/python3
import os
import sys
from http import cookies

def print_request_info():
    # Parse incoming cookies
    cookie = cookies.SimpleCookie()
    if 'HTTP_COOKIE' in os.environ:
        cookie.load(os.environ['HTTP_COOKIE'])
    
    # Create or get a session cookie
    if 'session_id' not in cookie:
        cookie['session_id'] = os.urandom(16).hex()
        cookie['session_id']['path'] = '/'
        cookie['session_id']['httponly'] = True
        cookie['session_id']['max-age'] = 3600  # 1 hour
    
    # Print headers
    print("Content-Type: text/html")
    print(cookie.output())
    print()  # Empty line to end headers
    
    # HTML content
    print("""<!DOCTYPE html>
<html>
<head>
    <title>CGI Request Information</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
        pre { background: #f5f5f5; padding: 10px; border-radius: 5px; }
        table { border-collapse: collapse; width: 100%; margin-bottom: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>""")
    
    print("<h1>CGI Request Information</h1>")
    
    # Cookie Information
    print("<h2>Cookie Information:</h2>")
    print("<table>")
    print("<tr><th>Cookie Name</th><th>Value</th></tr>")
    for key in cookie:
        print(f"<tr><td>{key}</td><td>{cookie[key].value}</td></tr>")
    print("</table>")
    
    # Environment Variables
    print("<h2>Environment Variables:</h2>")
    print("<pre>")
    for key in os.environ:
        print(f"{key}: {os.environ[key]}")
    print("</pre>")
    
    # Request Method
    print("<h2>Request Method:</h2>")
    print(f"<p>{os.environ.get('REQUEST_METHOD', 'Unknown')}</p>")
    
    # POST Data
    if os.environ.get('REQUEST_METHOD') == 'POST':
        try:
            content_length = int(os.environ.get('CONTENT_LENGTH', 0))
            if content_length > 0:
                post_data = sys.stdin.read(content_length)
                print("<h2>POST Data:</h2>")
                print("<pre>")
                print(post_data)
                print("</pre>")
        except ValueError:
            print("<p>Invalid CONTENT_LENGTH</p>")
    
    # Form to test cookies
    print("""
    <h2>Test Cookie</h2>
    <form method="POST" action="">
        <label for="cookie_name">Cookie Name:</label>
        <input type="text" id="cookie_name" name="cookie_name"><br>
        <label for="cookie_value">Cookie Value:</label>
        <input type="text" id="cookie_value" name="cookie_value"><br>
        <input type="submit" value="Set Cookie">
    </form>
    """)
    
    # Handle cookie setting from form
    if os.environ.get('REQUEST_METHOD') == 'POST':
        try:
            content_length = int(os.environ.get('CONTENT_LENGTH', 0))
            if content_length > 0:
                post_data = sys.stdin.read(content_length)
                if 'cookie_name=' in post_data and 'cookie_value=' in post_data:
                    params = dict(p.split('=') for p in post_data.split('&'))
                    cookie_name = params.get('cookie_name', '')
                    cookie_value = params.get('cookie_value', '')
                    if cookie_name:
                        cookie[cookie_name] = cookie_value
                        cookie[cookie_name]['path'] = '/'
                        print(f"<p>Cookie '{cookie_name}' set successfully!</p>")
        except Exception as e:
            print(f"<p>Error processing cookies: {str(e)}</p>")
    
    print("</body></html>")

if __name__ == "__main__":
    print_request_info()