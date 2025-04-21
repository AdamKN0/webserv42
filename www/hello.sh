#!/bin/bash

echo -ne "Content-Type: text/html\r\n"

# Create a new session_id cookie if not present
cookie_header=""
if [[ -z "$HTTP_COOKIE" || "$HTTP_COOKIE" != *"session_id="* ]]; then
    session_id=$(head /dev/urandom | tr -dc A-F0-9 | head -c 32)
    cookie_header="Set-Cookie: session_id=$session_id; Path=/; HttpOnly; Max-Age=3600"
    echo -ne "$cookie_header\r\n"
fi

# Handle POST data
if [[ "$REQUEST_METHOD" == "POST" ]]; then
    read -n "$CONTENT_LENGTH" POST_DATA
    IFS='&' read -ra params <<< "$POST_DATA"
    for param in "${params[@]}"; do
        key="${param%%=*}"
        val="${param#*=}"
        key=$(printf '%b' "${key//%/\\x}")
        val=$(printf '%b' "${val//%/\\x}")
        case "$key" in
            cookie_name) cookie_name="$val" ;;
            cookie_value) cookie_value="$val" ;;
        esac
    done

    if [[ -n "$cookie_name" && -n "$cookie_value" ]]; then
        echo -ne "Set-Cookie: $cookie_name=$cookie_value; Path=/\r\n"
    fi
fi

# Final header line
echo -ne "\r\n"

# Start HTML
cat <<EOF
<!DOCTYPE html>
<html>
<head>
    <title>CGI Request Info (Bash)</title>
    <style>
        body { font-family: Arial; max-width: 800px; margin: auto; padding: 20px; }
        pre { background: #f4f4f4; padding: 10px; overflow-x: auto; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ccc; padding: 8px; text-align: left; }
        th { background: #f0f0f0; }
    </style>
</head>
<body>
    <h1>CGI Request Info (Bash)</h1>
EOF

# Show cookies
echo "<h2>Cookies:</h2>"
ALL_COOKIES="$HTTP_COOKIE"
if [[ -n "$cookie_name" && -n "$cookie_value" ]]; then
    ALL_COOKIES="$ALL_COOKIES; $cookie_name=$cookie_value"
fi

if [[ -n "$ALL_COOKIES" ]]; then
    echo "<table><tr><th>Cookie Name</th><th>Value</th></tr>"
    echo "$ALL_COOKIES" | tr ';' '\n' | while IFS='=' read -r name val; do
        name=$(echo "$name" | xargs) # trim spaces
        val=$(echo "$val" | xargs)
        [[ -n "$name" ]] && echo "<tr><td>$name</td><td>$val</td></tr>"
    done
    echo "</table>"
else
    echo "<p>No cookies found.</p>"
fi

# Show environment
echo "<h2>Environment Variables:</h2><pre>"
printenv | sort
echo "</pre>"

# Show POST data
if [[ "$REQUEST_METHOD" == "POST" ]]; then
    echo "<h2>POST Data:</h2><pre>"
    for param in "${params[@]}"; do
        key="${param%%=*}"
        val="${param#*=}"
        key=$(printf '%b' "${key//%/\\x}")
        val=$(printf '%b' "${val//%/\\x}")
        echo "$key: $val"
    done
    echo "</pre>"
fi

# Form to set cookie
cat <<EOF
<h2>Set a Cookie</h2>
<form method="POST">
    <label>Cookie Name: <input type="text" name="cookie_name"></label><br>
    <label>Cookie Value: <input type="text" name="cookie_value"></label><br>
    <input type="submit" value="Set Cookie">
</form>
</body>
</html>
EOF
