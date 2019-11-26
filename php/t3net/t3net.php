<?php

/* helper function for sanitizing inputs */
function t3net_xml_escape($str)
{
	$str = str_replace("&", "&", $str);
	$str = str_replace("<", "<", $str);
	$str = str_replace(">", "&gt;", $str);
	$str = str_replace("\"", "&quot;", $str);
	return $str;
}

/* connect to the specified database and switch to UTF-8 encoding */
function t3net_connect($db_host, $db_user, $db_pass, $db_name)
{
	$linkID = mysql_connect($db_host, $db_user, $db_pass, $db_name) or die("Error: Could not connect to host.\r\n");
	mysql_set_charset('utf8', $linkID);

	return $linkID;
}

/* select a database */
function t3net_select_database($linkID, $db_database)
{
	mysql_select_db($db_database, $linkID) or die("Error: Could not find database.\r\n");
}

/* helper for creating an SQL query */
function t3net_create_sql_query($db_name, $command, $dummy_option)
{
	$query = $command . " " . $dummy_option;

	return query;
}

/* add an option to an SQL query */
function t3net_add_sql_query_option(&$query, $option, $separator)
{
	$query = $query . $separator . $option;

	return query;
}

/* execute the SQL query */
function t3net_query($linkID, $query)
{
	return mysql_query($query, $linkID) or die("Error: Data not found.\r\n");
}

/* helper for creating output in the correct format for T3Net native code
   to understand */
function t3net_create_output($header)
{
	$output = $header . "\r\n\r\n";

	return $output;
}

/* add an entry to output in the correct format */
function t3net_add_output_entry(&$output, $entry_name, $entry_val)
{
	$output = $output . "\t" . $entry_name . ": " . $entry_val . "\r\n";
}

/* finalize the output for sending to the native code */
function t3net_finalize_output(&$output)
{
	$output = $output . "\r\n";
}

?>
