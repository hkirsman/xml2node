<?php
// $Id$

/**
 * @file
 * maps original content to drupal content type fields
 *
 * @author Jan Azzati - Previon AG
 */
class Contentmapper {

  // property declarations

  // array - content read from file
  private $originalContent = array();
  // array - mapped content in array structure
  private $mappedContent = array();

  // constructor
  public function __construct($originalContent = array()) {
    $this->originalContent = $originalContent;
  }

  /**
   * returns the $mappedContent property
   */
  public function getNodes() {
    return $this->mappedContent;
  }

  /**
   * converts the original array structure into an array
   * that can be used to create drupal nodes
   */
  public function mapContent() {
    $count = count($this->originalContent);
    for ($i=0; $i < $count; $i++) {
      $this->mappedContent[$i] = array();
      $this->generateMappedContent($this->originalContent[$i], $this->mappedContent[$i]);
    }
  }

  /**
   * recursive method that converts the original array structure into
   * an array that is useful for creating drupal nodes
   *
   * @param $arrOriginal original array element
   * @param $arrMapped mapped array element
   */
  private function generateMappedContent($arrOriginal, &$arrMapped) {
    $arrMapped = array(
      'type' => $arrOriginal['@name'],
      'crud' => array(
        'action' => isset($arrOriginal['@attributes']['action']) ? $arrOriginal['@attributes']['action'] : 'c',
        'key' => isset($arrOriginal['@attributes']['key']) ? $arrOriginal['@attributes']['key'] : '',
        'value' => isset($arrOriginal['@attributes']['value']) ? $arrOriginal['@attributes']['value'] : '',
      ),
    );

    // if the created node must be referenced in a field, set the field name
    if (isset($arrOriginal['@attributes']['ref'])) {
      $arrMapped['reference_field'] = $arrOriginal['@attributes']['ref'];
    }

    // fill up the values array
    foreach ($arrOriginal['@children'] as $field) {
      if ($field['@attributes']['type'] == 'field') {
        $this->generateField($arrMapped['values'], $field, $arrMapped['type']);
      }
      elseif ($field['@attributes']['type'] == 'taxonomy') {
        $this->generateTaxonomy($arrMapped['values'], $field, $arrMapped['type']);
      }
      elseif ($field['@attributes']['type'] == 'contenttype') {
        $this->generateMappedContent($field, $arrMapped['references'][]);
        // define the field in the values array, so it will not be populated with default values
        if (isset($field['@attributes']['ref'])) {
          $arrMapped['values'][$field['@attributes']['ref']] = '';
        }
      }
      elseif ($field['@attriutes']['type'] == 'reference') {
        $this->generateReferenceField($arrMapped['values'], $field, $arrMapped['type']);
      }
    }
    
    // only fill missing fields with default values, when CRUD Action is 'create'
    // if we do this on 'update', some fields could be overwritten with default-values
    if ($arrMapped['crud']['action'] == 'c') {
      // complete missing fields with default value
      $this->fillOtherFieldsWithDefaultValue($arrMapped['values'], $arrMapped['type']);
    }
  }

  /**
   * generates the field value in the correct drupal values format
   *
   * @param $id the field name
   * @param $values the fields values
   */
  private function generateField(&$values, $field, $type = '') {
    $id = $field['@name'];
    // fill up the fields data in the values array
    if (isset($field['@children']) && count($field['@children']) > 0) {
      if ($field['@attributes']['type'] == 'field') {
        $values_ref = &$values[$id][];
      }
      else {
        $values_ref = &$values[$id];
      }
      
      foreach ($field['@children'] as $child) {
          $this->generateField($values_ref, $child);
      }
    }
    else { // just return the text value
      $values[$id] = $field['@text'];
    }
  }

  /**
   * generates the field value in the correct drupal values format
   * tries to get the node that matches the key value pair and 
   * references the nid to the field
   *
   * @param $values the node values array
   * @param $field the field 
   * @param $type the content type for the current node
   */
  private function generateReferenceField(&$values, $field, $type = '') {
    $id = $field['@name'];
    
    if (isset($field['@children']) && count($field['@children']) > 0) {
      if ($field['@attributes']['type'] == 'field') {
        $values_ref = &$values[$id][];
      }
      else {
        $values_ref = &$values[$id];
      }
      
      foreach ($field['@children'] as $child) {
          $this->generateReferenceField($values_ref, $child);
      }
    }
    else { // just return the text value
      if (isset($field['@attributes']['ref_key'])) {
        module_load_include('inc', 'xml2node', 'includes/xml2node.nodecreator');
        $node = Nodecreator::getNode($type ,$field['ref_key'], $field['@text']);
        $values[$id] = isset($node->nid) ? $node->nid : '';
      }
      else {
        // TODO: log that the argument is missing
      }
    }
  }

  /**
   * completes the values array with default values of
   * the missing fields
   *
   * @param $valuesArray array - values array
   * @param $type string - content type
   */
  private function fillOtherFieldsWithDefaultValue(&$valuesArray, $type) {
    // get contenttype information
    $contentTypeInfo = content_types($type);

    // go through all the fields of the content_type
    foreach ($contentTypeInfo['fields'] as $field) {
      if (!isset($valuesArray[$field['field_name']])) {
        $valuesArray[$field['field_name']] = $this->cckDefaultValueHandler($field['field_name'], $type);
      }
    }
  }

  /**
   * returns a cck fields default value
   *
   * @param $id string - the cck field
   * @param $type string - the content type
   */
  private function cckDefaultValueHandler($id, $type) {
    // get contenttype information
    $contentTypeInfo = content_types($type);

    if (isset($contentTypeInfo['fields'][$id]['widget']['default_value'])) {
      if ($contentTypeInfo['fields'][$id]['type'] != 'datestamp' &&
      $contentTypeInfo['fields'][$id]['type'] != 'datetime') {
        return $contentTypeInfo['fields'][$id]['widget']['default_value'];
      }
    }
  }

  /**
   * adds the taxonomy terms into the values array
   *
   * @param array $values the values array
   * @param array $field the taxonomy term array
   * @param string $type the content type (not used yet)
   */
  private function generateTaxonomy(&$values, $field, $type = '') {
    foreach ($field['@children'] as $term) {
      if ($term['@name'] == 'term' && isset($term['@attributes']['vid'])) {
        $this->buildTaxonomyTree($values, $term, 0);
      }
    }
  }

  /**
   * gets the term id of the given term or creates it if it doesn't exist
   * takes also care of the hierachical structure of the terms
   *
   * @param $values array - the values array
   * @param $term string - the term name
   * @param $parentId - the tid of the parent term
   */
  private function buildTaxonomyTree(&$values, $term, $parentId=0) {
    $termId = $this->getTermByName($term['@text'], $term['@attributes']['vid']);

    if (!$termId) {
      $termId = $this->saveTaxonomyTerm($term['@text'], $term['@attributes']['vid'], $parentId);
    }
    
    // Has this taxonomie child terms
    if (isset($term['@children']) && count($term['@children']) > 0) {
      // call this function for every child, with the current term id as parent id
      foreach ($term['@children'] as $childTerm) {
        if ($childTerm['@name'] == 'term' && isset($childTerm['@attributes']['vid'])) {
          $this->buildTaxonomyTree($values, $childTerm, $termId);
        }
      }
    }
    else {
      // we are finished, set the termId into the values field
      $values['taxonomy'][$term['@attributes']['vid']][] = $termId;
    }
    
    // Build parent "path"
    if (isset($term['@attributes']['take_parents']) && $term['@attributes']['take_parents'] == '1') {
      
      $parents = taxonomy_get_parents_all($termId);
      
      foreach ($parents as $key => $parent) {
        // Ignore first element because it's the actual term
        if ($key > 0) {
          $values['taxonomy'][$term['@attributes']['vid']][] = $parent->tid;
        }
      }
    }
  }

  /**
   * gets the term id of a taxonomy term with the corresponding name
   * and the corrsponding vocabulary id and returns it
   *
   * @param string $term the term name
   * @param int $vid the vocabulary id
   *
   * @return term id of the term or null, if the term wasn't found
   */
  private function getTermByName($term, $vid) {
    $terms = taxonomy_get_term_by_name($term);
    foreach ($terms as $currTerm) {
      if ($currTerm->vid == $vid) {
        return $currTerm->tid;
      }
    }
    return NULL;
  }

  /**
   * saves a taxonomy term in the database table
   *
   * @param $termName string - the name of the term
   * @param $vid int - the id of the vocab
   * @param $parentId int - the id of the parent term
   */
  private function saveTaxonomyTerm($termName, $vid, $parentId = 0) {
    $term = array(
      'vid' => $vid,             // Voacabulary ID
      'name' => $termName,       // Term Name 
    //'synonyms' => 'Druplet',   // (Optional) Synonym of this term
    //'relations' => array(15),  // (Optional) Related Term IDs
    );

    if ($parentId > 0) {
      $term['parent'] = $parentId; // (Optional) Term ID of a parent term
    }

    // save the term
    taxonomy_save_term($term);

    return $term['tid'];
  }

  // destructor
  public function __destruct() {
    // not used - placeholder
  }
}