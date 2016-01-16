<?php

/********************************
** Template for use with T3Net **
********************************/

/* database info */
$db_host = "0.0.0.0"; // address of host
$db_user = "username"; // login credentials
$db_pass = "password";
$db_database = "database"; // root database
$db_name = "name"; // name of our database (database->name->fields)
$db_fields = array('field_1', 'field_2', 'field_3'); // fields we are interested in

/* command-specific data */

/* connect to database */
$linkID = mysql_connect($db_host, $db_user, $db_pass) or die("Error: Could not connect to host.\r\n");
mysql_select_db($db_database, $linkID) or die("Error: Could not find database.\r\n");

/* build query from passed fields */
$query = "INSERT INTO " . $db_name . " SET dummy = '66'";
foreach($db_fields as $field)
{
	if(strlen($_GET[$field]) > 0)
	{
		$query .= ", " . mysql_real_escape_string($field) . " = '" . mysql_real_escape_string($_GET[$field]) . "'";
	}
}

/* command-specific query options */

/* run the query */
$result = mysql_query($query, $linkID) or die("Error: Invalid Entry.\r\n");
if(mysql_affected_rows() == 0)
{
	// this most likely won't happen
	print "Error: Invalid Request\r\n";
	exit;
}

/* success */
print "ack:\r\n";

?>
