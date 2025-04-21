#!/usr/bin/perl
use strict;
use warnings;
use CGI;
use CGI::Cookie;

# Create CGI object
my $cgi = CGI->new;

# Parse cookies
my %cookies = parse CGI::Cookie;

# Create or get session cookie
unless (exists $cookies{'session_id'}) {
    my $session_id = join '', map { unpack "H*", chr(rand(256)) } 1..16;
    my $cookie = CGI::Cookie->new(
        -name     => 'session_id',
        -value    => $session_id,
        -path     => '/',
        -httponly => 1,
        -max_age  => 3600
    );
    print "Set-Cookie: $cookie\n";
    $cookies{'session_id'} = $cookie;
}

# Print headers
print $cgi->header('text/html');

# HTML content
print <<"HTML";
<!DOCTYPE html>
<html>
<head>
    <title>CGI Request Information (Perl)</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
        pre { background: #f4f4f4; padding: 10px; overflow-x: auto; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h1>CGI Request Information (Perl)</h1>
HTML

# Cookie Information
print "<h2>Cookies:</h2>\n";
if (keys %cookies) {
    print "<table>\n";
    print "<tr><th>Cookie Name</th><th>Value</th></tr>\n";
    foreach my $name (keys %cookies) {
        my $value = ref($cookies{$name}) ? $cookies{$name}->value : $cookies{$name};
        print "<tr><td>$name</td><td>$value</td></tr>\n";
    }
    print "</table>\n";
} else {
    print "<p>No cookies found.</p>\n";
}

# Environment Variables
print "<h2>Environment Variables:</h2>\n";
print "<pre>\n";
foreach my $key (sort keys %ENV) {
    print "$key: $ENV{$key}\n";
}
print "</pre>\n";

# POST Data
if ($ENV{'REQUEST_METHOD'} eq 'POST') {
    print "<h2>POST Data:</h2>\n";
    print "<pre>\n";
    my %params = $cgi->Vars;
    foreach my $key (sort keys %params) {
        print "$key: $params{$key}\n";
    }
    print "</pre>\n";
}

# Form to set cookies
print <<"HTML";
    <h2>Set a Cookie</h2>
    <form method="POST">
        <label>Cookie Name: <input type="text" name="cookie_name"></label><br>
        <label>Cookie Value: <input type="text" name="cookie_value"></label><br>
        <input type="submit" value="Set Cookie">
    </form>
</body>
</html>
HTML

# Handle cookie setting from form
if ($ENV{'REQUEST_METHOD'} eq 'POST') {
    my $cookie_name = $cgi->param('cookie_name');
    my $cookie_value = $cgi->param('cookie_value');
    
    if ($cookie_name) {
        my $cookie = CGI::Cookie->new(
            -name  => $cookie_name,
            -value => $cookie_value,
            -path  => '/'
        );
        print "Set-Cookie: $cookie\n";
        print "<p>Cookie '$cookie_name' set successfully!</p>\n";
    }
}