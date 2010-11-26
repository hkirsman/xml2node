<?php
// $Id$

/**
 * @file
 * simplifies the process of creating drupal 6 nodes programmatically
 *
 * @author Jan Azzati - Previon AG
 */
class Nodecreator {

  // property declarations
  // ...

  // constructor
  public function __construct() {
  }

  /**
   * DON'T USE THIS FUNCTION. IT IS NOT UP TO DATE --> USE nodeSaveAlternative
   * generic method to create a node in drupal
   *
   * @param $type string - node type to create (required)
   * @param $language string - language of the content
   * @param $values array - all field values for the node
   *
   * @return node id of the created node, or false if there were some errors
   */
  public function saveNode($type, $language = '', $values = array()) {
    if (!empty($type)) {
      global $user;

      // import node functions
      module_load_include('inc', 'node', 'node.pages');

      // create basic node structure
      // TODO: fill basic fields also from the $values array
      $node = array(
        'uid' => (string) $user->uid,
        'name' => (string) $user->name,
        'type' => $type,
        'language' => $language,
        'body' => NULL,
        'title' => NULL,
        'format' => NULL,
        'status' => TRUE,
        'promote' => FALSE,
        'sticky' => FALSE,
      // TODO: get created and changed date out of the values array
        'created' => time(),
        'revision' => FALSE,
        'comment' => '0',
      );

      $node = (object) $node;

      // set form state values (content)
      $form_state['values'] = $values;

      $form_state['values']['name'] = $node->name;
      $form_state['values']['op'] = t('Save');

      // drupal needs that, why? -> http://drupal.org/node/293663
      // http://drupal.org/node/726868
      $form_state['node'] = $form_state['values'];

      // run drupal_execute to follow the normal drupal process to save the node
      drupal_execute($type . '_node_form', $form_state, $node);

      if (isset($form_state['nid'])) {
        return $form_state['nid'];
      }
    }
    return FALSE;
  }

  /**
   * generic method to save a node into drupal using node_save
   *
   * @param $type string - the content type
   * @param $language string - the language (optional)
   * @param $values array - the values array
   * @param $crud array - the CRUD Action to perform
   */
  public function nodeSaveAlternative($type, $language = '', $values = array(), $crud = array('action' => 'c')) {
    if (!empty($type)) {
      global $user;
      
      $node = new stdClass();
      
      switch ($crud['action']) {
        case 'u':
          // UPDATE
          $node = $this->getNode($type, $crud['key'], $crud['value']);
        case 'c':
          // CREATE / UPDATE
          if (!isset($node->nid)) {
            $node = new stdClass();
      
            // create basic node structure
            $node->uid = (string) $user->uid;
            $node->name = (string) $user->name;
            $node->type = $type;
            $node->language = $language;
            $node->body = NULL;
            $node->title = NULL;
            $node->format = NULL;
            $node->status = TRUE;
            $node->promote = FALSE;
            $node->sticky = FALSE;
            $node->created = time();
            $node->revision = FALSE;
            $node->comment = 0;
          }
          
          foreach ($values as $fieldName => $value) {
            $node->$fieldName = $value;
          }
    
          node_save($node);
    
          if (isset($node->nid)) {
            return $node->nid;
          }
          break;
        case 'd':
          // DELETE
          $node = $this->getNode($type, $crud['key'], $crud['value']);
          if (isset($node->nid)) {
            node_delete($node->nid);
          }
          break;
      }
      
      
    }
    return FALSE;
  }
  
  /**
   * Loads a node that fullfills the key=>value condition
   * 
   * @param string $key
   * @param mixed $value
   * @return loaded node
   */
  private function getNode($type, $key = NULL, $value = NULL) {
    $sql = "SELECT nid FROM {content_type_%s} WHERE %s='%s'";
    $result = db_query($sql, $type, $key, $value);
    $node = db_fetch_object($result);
    
    if (is_object($node) && isset($node->nid)) {
      $node = node_load($node->nid);
    }
    else {
      $node = new stdClass();
    }
    
    return $node;
  }

  /**
   * checks if the $values array can be used to
   * create a node of the given type
   *
   * @param $type the node type
   * @param $values values to validate
   *
   * @return true if $values is valid
   */
  private function validateValues($type, $values) {
    $result = FALSE;
    //throw new Exception("not yet implemented");
    return $result;
  }

  // destructor
  public function __destruct() {
    // not needed - placeholder
  }
}