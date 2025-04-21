<?php
function parse_cgi_input() {
    // Read input from stdin based on CONTENT_LENGTH
    $content_length = isset($_SERVER['CONTENT_LENGTH']) ? intval($_SERVER['CONTENT_LENGTH']) : 0;
    $input = '';
    
    if ($content_length > 0) {
        // Read exact number of bytes specified by CONTENT_LENGTH
        $input = fread(STDIN, $content_length);
    }
    
    return $input;
}

function parse_cgi_cookies() {
    $cookies = [];
    
    // Check for COOKIE environment variable
    $cookie_str = $_SERVER['HTTP_COOKIE'] ?? $_ENV['HTTP_COOKIE'] ?? '';
    
    if (!empty($cookie_str)) {
        // Split cookie string into individual cookies
        $cookie_pairs = explode('; ', $cookie_str);
        
        foreach ($cookie_pairs as $cookie_pair) {
            $parts = explode('=', $cookie_pair, 2);
            if (count($parts) == 2) {
                $cookies[trim($parts[0])] = urldecode(trim($parts[1]));
            }
        }
    }
    
    return $cookies;
}

function print_request_info() {
    // Parse input
    $raw_input = parse_cgi_input();
    
    // Parse cookies
    $cookies = parse_cgi_cookies();
    
    // If no session cookie, create one
    if (!isset($cookies['session_id'])) {
        $cookies['session_id'] = bin2hex(random_bytes(16));
        // Set session cookie header
        header("Set-Cookie: session_id={$cookies['session_id']}; Path=/; HttpOnly; Max-Age=3600");
    }
    
    // Parse POST data if input exists
    $post_data = [];
    if (!empty($raw_input)) {
        parse_str($raw_input, $post_data);
    }
    
    // Output headers
    header('Content-Type: text/html');
    ?>
<!DOCTYPE html>
<html>
<head>
    <title>CGI Request Information</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
        pre { background: #f4f4f4; padding: 10px; overflow-x: auto; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    </style>
</head>
<body>
    <h1>CGI Request Information</h1>

    <h2>Cookies:</h2>
    <?php
    if (!empty($cookies)) {
        echo "<table>";
        echo "<tr><th>Cookie Name</th><th>Value</th></tr>";
        foreach ($cookies as $name => $value) {
            echo "<tr><td>" . htmlspecialchars($name) . "</td><td>" . htmlspecialchars($value) . "</td></tr>";
        }
        echo "</table>";
    } else {
        echo "<p>No cookies found.</p>";
    }
    ?>
    <h2>POST Data:</h2>
    <pre><?php 
        print_r($post_data);
    ?></pre>

    <h2>Set a Cookie</h2>
    <form method="POST">
        <label>Cookie Name: <input type="text" name="cookie_name"></label><br>
        <label>Cookie Value: <input type="text" name="cookie_value"></label><br>
        <input type="submit" value="Set Cookie">
    </form>

    <h2>Server Environment:</h2>
    <pre><?php
    foreach ($_SERVER as $key => $value) {
        echo htmlspecialchars($key) . ": " . 
             (is_array($value) 
                 ? print_r($value, true) 
                 : htmlspecialchars($value)) . "\n";
    }
    ?></pre>
</body>
</html>
<?php
}

print_request_info();
?>