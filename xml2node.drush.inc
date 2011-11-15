<?php

/**
 * Implements hook_drush_command().
 */
function xml2node_drush_command() {
  $items = array(
    'x2n-import' => array(
      'description' => 'Import an xml2node xml.',
      'arguments' => array(
        'files' => 'xml files to import (absolute path)',
      ),
      'options' => array(
        '--queue' => 'Only add the items to the queue, don\'t import them directly',
        '--count' => 'The number of items to import',
        '--offset' => 'The number of items to skip',
      ),
      'callback' => 'xml2node_drush_import',
    ),
    'x2n-queue' => array(
      'description' => 'Get queue status / clear the queue',
      'options' => array(
        'status' => 'Get queue status',
        'clear' => 'Clear the queue',
      ),
      'callback' => 'xml2node_drush_queue',
    ),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function xml2node_drush_help($section) {
  switch ($section) {
    case 'drush:x2n-import':
      return dt('Import xml files.');
    case 'drush:x2n-queue':
      return dt('Get the xml2node queue status or clear the queue.');
  }
}

/**
 * Callback for the x2n-import command
 *
 * Imports one or more xml files into drupal.
 */
function xml2node_drush_import($files) {
  // Get options based on drush call
  $method = drush_get_option('--queue') ? 'queue' : 'batch';
  $count = drush_get_option('--count', 0);
  $offset = drush_get_option('--offset', 0);

  // Import each file
  foreach ($files as $file) {
    drush_print(dt('Processing @file', array('@file' => $file)));

    xml2node_proceed_import($file, $count, $offset, $method, FALSE);

    if ($method == 'batch') {
      $batch =& batch_get();
      $batch['progressive'] = FALSE;

      drush_print(dt('Starting batch import for file @file', array('@file' => $file)));
      drush_backend_batch_process();
    }
  }

  drush_log(dt('Import completed. Check watchdog log!'), 'ok');
}

/**
 * Callback for the x2n-queue command
 *
 * Deletes the xml2node queue or returns the number
 * of items in the queue based on the options
 */
function xml2node_drush_queue() {
  $clear = drush_get_option('clear', 0);
  $status = drush_get_option('status', 0);

  if ($clear && !$status) {
    // clear the whole xml2node queue
    $queue = drupal_queue_get('xml2node_queue');
    $queue->deleteQueue();
  }
  else if ($status && !$clear) {
    // return number of items in the xml2node queue
    include_once('xml2node.settings.inc');
    $num_of_items = xml2node_show_queue_status();
    drush_print(dt('The queue contains @count items', array('@count' => $num_of_items)));
  }
  else {
    // the command options are invalid, return an error msg
    drush_print(dt('Wrong command parameters! You may use x2n-queue with "clear" OR "status" as options!'));
  }
}