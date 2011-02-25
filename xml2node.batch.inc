<?php

/*
 * @file
 */

/**
 * callback for the batch job to import a node
 * 
 * @param integer $index
 * @param string $index_file
 * @param array $context
 */
function xml2node_import_node($index, $import_file, &$context) {
  // import the xml record with the passed index from the passed file
  $result = xml2node_import_xml_record($index, $import_file);

  $context['results'][] = $result;
}

/**
 * callback for the batch job when its finished
 * 
 * @param $success
 * @param $results
 * @param $operations
 */
function xml2node_import_node_finished($success, $results, $operations) {
  
  if ($success) {
    $message = format_plural(count($results), '1 node imported!', '@count nodes imported!');
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
  }
  
  drupal_set_message($message);
}