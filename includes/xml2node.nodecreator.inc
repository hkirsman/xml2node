<?php

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
  public function nodeSaveAlternative($type, $language = '', $values = array(), $crud = array('action' => 'c'), $redirect = NULL) {
    if (!empty($type)) {
      global $user;

      $node = new stdClass();

      switch ($crud['action']) {
        case 'u':
          // UPDATE
          $node = self::getNode($type, $crud['key'], $crud['value']);
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
            $this->createRedirect($node->nid, $redirect, $crud['action']);
            return $node->nid;
          }
          else {
            // node wasn't saved correctly, log it
            watchdog('xml2node', 'Error saving node: !title'
                , array(
                  '!title' => $node->title,
                ), WATCHDOG_ERROR);
          }
          break;
        case 'd':
          // DELETE
          $node = self::getNode($type, $crud['key'], $crud['value']);
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
  public static function getNode($type, $key = NULL, $value = NULL) {
    if (isset($key) && isset($value) && isset($type)) {
      // get contenttype information
      $contentTypeInfo = content_types($type);
      $field = $contentTypeInfo['fields'][$key];

      if (isset($field) && is_array($field)) {
        if ($field['db_storage'] == 0) {
          $sql = "SELECT node.nid AS nid FROM {node} LEFT JOIN {content_%s} AS field_table ON node.vid = field_table.vid WHERE (node.type in ('%s')) AND ((field_table.%s_value) = ('%s'))";
          $result = db_query($sql, $key, $type, $key, $value);
          $node = db_fetch_object($result);
        }
        elseif ($field['db_storage'] == 1) {
          $sql = "SELECT nid FROM {content_type_%s} WHERE %s='%s'";
          $result = db_query($sql, $type, $key . '_value', $value);
          $node = db_fetch_object($result);
        }

        if (isset($node) && is_object($node) && isset($node->nid)) {
          $node = node_load($node->nid);
          return $node;
        }
        else {
          return null;
        }
      }
    }
    return null;
  }

  /**
   * creates a redirect for a specific url to the node
   * based on the path_redirect module (if enabled).
   * 
   * @param $nid the node id to redirect to
   * @param $redirect_url the source url
   * @param $crud_op u or c for update or create
   */
  private function createRedirect($nid, $redirect_url, $crud_op) {
    if (isset($redirect_url) && !empty($redirect_url) && isset($nid) && !empty($nid)) {
      if (module_exists('path_redirect')) {
        $redirect_obj = array(
          'source' => '',
          'redirect' => 'node/' . $nid,
          'language' => '',
          'type' => 301,
          'query' => '',
          'fragment' => '',
          'last_used' => time(),
        );

        $url = parse_url($redirect_url);
        $redirect_obj['query'] = $url['query'];
        $redirect_obj['fragment'] = $url['fragment'];
        $redirect_obj['source'] = $url['path'];

        switch ($crud_op) {
          case 'u':
          // TODO: Check if there is already a redirect for the source and update it.
          case 'c':
            $result = path_redirect_save($redirect_obj);

            if (!isset($result['rid'])) {
              watchdog('xml2node', 'Redirect for node !node not created. URL: !url'
                  , array(
                '!node' => $nid,
                '!url' => $redirect_url,
                  ), WATCHDOG_ERROR);
            }

            break;
        }
      }
    }
  }

  /**
   * saves a comment referring to a node or it's parent comment
   * @param $comment the comment values
   * @param $crud
   */
  public function commentSave($comment, $crud) {
    if (isset($comment) && count($comment) > 0 && ($crud == 'c' || $crud == 'u')) {
      if (module_exists('comment')) {
        $cid = comment_save($comment);
        if ($cid === false) {
          watchdog('xml2node', 'Comment for node !node not created'
              , array(
            '!node' => $comment['nid'],
              ), WATCHDOG_ERROR);
        }
        else {
          return $cid;
        }
      }
    }

    return FALSE;
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