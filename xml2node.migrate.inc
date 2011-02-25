<?php

/*
 * @file
 */

/**
 * form for processing the import
 */
function xml2node_migrate_form() {
  $form = array();

  // field for the filepath
  $form['xml2node_filepath'] = array(
    '#type' => 'textfield',
    '#title' => t('XML File'),
    '#default_value' => variable_get('xml2node_filepath', 'sites/default/files/'),
    '#size' => 60,
    '#maxlength' => 250,
    '#description' => t('The path to the XML File you would like to import.'),
  );
  
  $form['xml2node_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of elements'),
    '#default_value' => variable_get('xml2node_count', 0),
    '#size' => 4,
    '#maxlength' => 4,
    '#description' => t('The number of elements that should be imported.'),
  );
  
  $form['xml2node_offset'] = array(
    '#type' => 'textfield',
    '#title' => t('offset'),
    '#default_value' => variable_get('xml2node_offset', 0),
    '#size' => 4,
    '#maxlength' => 4,
    '#description' => t('The number of elements that should be skipped.'),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Import'));

  return $form;
}

/**
 * Implements hook_form_submit().
 */
function xml2node_migrate_form_submit($form, &$form_state) {
  if (isset($form['xml2node_filepath']) && !empty($form['xml2node_filepath']['#value'])) {
    // save the field values into variables, so they are present the next time the form is called
    variable_set('xml2node_filepath', $form['xml2node_filepath']['#value']);
    variable_set('xml2node_count', $form['xml2node_count']['#value']);
    variable_set('xml2node_offset', $form['xml2node_offset']['#value']);
    
    $filepath = $form['xml2node_filepath']['#value'];
    $count = is_numeric($form['xml2node_count']['#value']) ? $form['xml2node_count']['#value'] : 0;
    $offset = is_numeric($form['xml2node_offset']['#value']) ? $form['xml2node_offset']['#value'] : 0;
    
    // proceed the node import from the xml file
    xml2node_proceed_import($filepath, $count, $offset);
  }
}