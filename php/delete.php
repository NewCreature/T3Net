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
$linkID = mysql_connect($db_host, $db_user, $db_pass) or die("Error: Could not connect to host.");
mysql_select_db($db_database, $linkID) or die("Error: Could not find database.");

/* build query */
$query = "DELETE FROM servers WHERE dummy=66'";

/* update fields passed as arguments */
foreach($db_fields as $field)
{
	if(strlen($_GET[$field]) > 0)
	{
		$query .= "AND " . mysql_real_escape_string($field) . " = '" . mysql_real_escape_string($_GET[$field]) . "'";
	}
}

/* add other stuff to query here */


if(mysql_query($query, $linkID) && mysql_affected_rows() == 0)
{
	print 'Error: Invalid Request';
}
print 'ack:';

?>