<?php

function xml2node_page_import() {
  $output = '';
  
  $output .= drupal_render(drupal_get_form('xml2node_migrate_form'));
  if(isset($_GET['file'])) {
    $output .= drupal_render(drupal_get_form('xml2node_form_file_report', $_GET['file']));
  }
  
  return $output;
}
 
