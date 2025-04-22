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
    
    # HTML content
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>System Information Dashboard</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --primary: #2563eb;
            --primary-dark: #1d4ed8;
            --secondary: #0ea5e9;
            --accent: #6366f1;
            --dark: #0f172a;
            --dark-light: #1e293b;
            --light: #f8fafc;
            --gray: #64748b;
            --gray-light: #e2e8f0;
            --success: #10b981;
            --warning: #f59e0b;
            --danger: #ef4444;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Inter', sans-serif;
            background-color: var(--dark);
            color: var(--light);
            line-height: 1.6;
            width: 1200px;
            margin: 0 auto;
            padding: 40px;
        }
        
        .dashboard {
            display: flex;
            flex-direction: column;
            gap: 30px;
        }
        
        .header {
            background: linear-gradient(135deg, var(--primary), var(--accent));
            padding: 40px;
            border-radius: 12px;
            margin-bottom: 20px;
            position: relative;
            overflow: hidden;
            box-shadow: 0 10px 25px rgba(37, 99, 235, 0.2);
        }
        
        .header::before {
            content: '';
            position: absolute;
            top: -50%;
            right: -20%;
            width: 600px;
            height: 600px;
            background: radial-gradient(circle, rgba(255,255,255,0.1) 0%, rgba(255,255,255,0) 70%);
            border-radius: 50%;
        }
        
        .header h1 {
            font-size: 36px;
            font-weight: 700;
            margin-bottom: 10px;
            position: relative;
            z-index: 2;
        }
        
        .header p {
            font-size: 16px;
            opacity: 0.9;
            max-width: 600px;
            position: relative;
            z-index: 2;
        }
        
        .card {
            background: var(--dark-light);
            border-radius: 12px;
            padding: 30px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.05);
        }
        
        .card-header {
            display: flex;
            align-items: center;
            margin-bottom: 20px;
            padding-bottom: 15px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        .card-icon {
            width: 40px;
            height: 40px;
            border-radius: 8px;
            display: flex;
            align-items: center;
            justify-content: center;
            margin-right: 15px;
            background: rgba(37, 99, 235, 0.2);
            color: var(--primary);
        }
        
        .card-title {
            font-size: 20px;
            font-weight: 600;
            color: var(--light);
            margin: 0;
        }
        
        table {
            width: 100%;
            border-collapse: separate;
            border-spacing: 0;
            margin: 15px 0;
            border-radius: 8px;
            overflow: hidden;
        }
        
        th, td {
            padding: 12px 15px;
            text-align: left;
        }
        
        th {
            background: rgba(37, 99, 235, 0.1);
            font-weight: 500;
            color: var(--light);
            font-size: 14px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        tr {
            background: rgba(30, 41, 59, 0.5);
        }
        
        tr:nth-child(even) {
            background: rgba(30, 41, 59, 0.8);
        }
        
        pre {
            background: rgba(15, 23, 42, 0.7);
            color: var(--secondary);
            padding: 20px;
            border-radius: 8px;
            overflow-x: auto;
            font-family: monospace;
            font-size: 14px;
            line-height: 1.5;
            border: 1px solid rgba(255, 255, 255, 0.05);
        }
        
        form {
            display: flex;
            flex-direction: column;
            gap: 15px;
        }
        
        .form-group {
            display: flex;
            flex-direction: column;
            gap: 8px;
        }
        
        label {
            font-weight: 500;
            color: var(--light);
            font-size: 14px;
        }
        
        input[type="text"] {
            padding: 12px 15px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            background: rgba(15, 23, 42, 0.7);
            border-radius: 8px;
            font-size: 14px;
            font-family: 'Inter', sans-serif;
            color: var(--light);
            transition: all 0.3s ease;
        }
        
        input[type="text"]:focus {
            outline: none;
            border-color: var(--primary);
            box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.2);
        }
        
        input[type="submit"] {
            padding: 12px 20px;
            background: var(--primary);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s ease;
            margin-top: 10px;
            align-self: flex-start;
        }
        
        input[type="submit"]:hover {
            background: var(--primary-dark);
        }
        
        .home-button {
            position: fixed;
            bottom: 30px;
            right: 30px;
            background: var(--primary);
            color: white;
            width: 60px;
            height: 60px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 4px 10px rgba(37, 99, 235, 0.3);
            text-decoration: none;
            transition: all 0.3s ease;
        }
        
        .home-button:hover {
            background: var(--primary-dark);
            transform: translateY(-3px);
            box-shadow: 0 6px 15px rgba(37, 99, 235, 0.4);
        }
        
        .home-icon {
            width: 24px;
            height: 24px;
        }
        
        .grid-bg {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-image: 
                linear-gradient(rgba(37, 99, 235, 0.03) 1px, transparent 1px),
                linear-gradient(90deg, rgba(37, 99, 235, 0.03) 1px, transparent 1px);
            background-size: 20px 20px;
            z-index: -1;
        }
        
        .method-badge {
            display: inline-block;
            padding: 6px 12px;
            background: rgba(37, 99, 235, 0.2);
            color: var(--primary);
            border-radius: 6px;
            font-weight: 500;
            font-size: 14px;
            letter-spacing: 0.5px;
        }
        
        .success-message {
            background: rgba(16, 185, 129, 0.1);
            border-left: 4px solid var(--success);
            padding: 10px 15px;
            margin: 15px 0;
            color: var(--success);
            border-radius: 4px;
        }
        
        .error-message {
            background: rgba(239, 68, 68, 0.1);
            border-left: 4px solid var(--danger);
            padding: 10px 15px;
            margin: 15px 0;
            color: var(--danger);
            border-radius: 4px;
        }
    </style>
</head>
<body>
    <div class="grid-bg"></div>
    
    <div class="dashboard">
        <div class="header">
            <h1>System Information Dashboard</h1>
            <p>View and manage your session information, cookies, and environment variables</p>
        </div>
        
        <div class="card">
            <div class="card-header">
                <div class="card-icon">
                    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                        <path d="M12 2a10 10 0 1 0 10 10 4 4 0 0 1-5-5 4 4 0 0 1-5-5"></path>
                        <path d="M8.5 8.5v.01"></path>
                        <path d="M16 15.5v.01"></path>
                        <path d="M12 12v.01"></path>
                        <path d="M11 17v.01"></path>
                        <path d="M7 14v.01"></path>
                    </svg>
                </div>
                <h2 class="card-title">Cookie Information</h2>
            </div>
            <table>
                <tr>
                    <th>Cookie Name</th>
                    <th>Value</th>
                </tr>""")
    
    # Cookie Information
    for key in cookie:
        print(f"<tr><td>{key}</td><td>{cookie[key].value}</td></tr>")
    
    print("""
            </table>
        </div>
        
        <div class="card">
            <div class="card-header">
                <div class="card-icon">
                    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                        <path d="M20 7h-9"></path>
                        <path d="M14 17H5"></path>
                        <circle cx="17" cy="17" r="3"></circle>
                        <circle cx="7" cy="7" r="3"></circle>
                    </svg>
                </div>
                <h2 class="card-title">Environment Variables</h2>
            </div>
            <pre>""")
    
    # Environment Variables
    for key in sorted(os.environ.keys()):
        print(f"{key}: {os.environ[key]}")
    
    print("""</pre>
        </div>
        
        <div class="card">
            <div class="card-header">
                <div class="card-icon">
                    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                        <rect width="18" height="18" x="3" y="3" rx="2"></rect>
                        <path d="M7 7h10"></path>
                        <path d="M7 12h10"></path>
                        <path d="M7 17h10"></path>
                    </svg>
                </div>
                <h2 class="card-title">Request Method</h2>
            </div>
            <div class="method-badge">""")
    
    # Request Method
    print(f"{os.environ.get('REQUEST_METHOD', 'Unknown')}")
    
    print("""</div>
        </div>""")
    
    # POST Data
    if os.environ.get('REQUEST_METHOD') == 'POST':
        try:
            content_length = int(os.environ.get('CONTENT_LENGTH', 0))
            if content_length > 0:
                post_data = sys.stdin.read(content_length)
                print("""
        <div class="card">
            <div class="card-header">
                <div class="card-icon">
                    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                        <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path>
                        <polyline points="17 8 12 3 7 8"></polyline>
                        <line x1="12" y1="3" x2="12" y2="15"></line>
                    </svg>
                </div>
                <h2 class="card-title">POST Data</h2>
            </div>
            <pre>""")
                print(post_data)
                print("""</pre>
        </div>""")
        except ValueError:
            print("""
        <div class="error-message">Invalid CONTENT_LENGTH</div>""")
    
    # Form to test cookies
    print("""
        <div class="card">
            <div class="card-header">
                <div class="card-icon">
                    <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                        <path d="M12 2a10 10 0 1 0 10 10 4 4 0 0 1-5-5 4 4 0 0 1-5-5"></path>
                        <path d="M8.5 8.5v.01"></path>
                        <path d="M16 15.5v.01"></path>
                        <path d="M12 12v.01"></path>
                        <path d="M11 17v.01"></path>
                        <path d="M7 14v.01"></path>
                    </svg>
                </div>
                <h2 class="card-title">Set a Cookie</h2>
            </div>
            <form method="POST" action="">
                <div class="form-group">
                    <label for="cookie_name">Cookie Name:</label>
                    <input type="text" id="cookie_name" name="cookie_name" placeholder="Enter cookie name">
                </div>
                <div class="form-group">
                    <label for="cookie_value">Cookie Value:</label>
                    <input type="text" id="cookie_value" name="cookie_value" placeholder="Enter cookie value">
                </div>
                <input type="submit" value="Set Cookie">
            </form>""")
    
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
                        print(f"""
            <div class="success-message">Cookie '{cookie_name}' set successfully!</div>""")
        except Exception as e:
            print(f"""
            <div class="error-message">Error processing cookies: {str(e)}</div>""")
    
    print("""
        </div>
    </div>
    
    <a href="/" class="home-button">
        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="home-icon">
            <path d="m3 9 9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"></path>
            <polyline points="9 22 9 12 15 12 15 22"></polyline>
        </svg>
    </a>
</body>
</html>""")

if __name__ == "__main__":
    print_request_info()