<?php
// $Id$

/**
 * @file
 * parses conent from a xml file for usage in the later process
 *
 * @author Jan Azzati - Previon AG
 */
class Contentparser {

  // property declarations

  // XMLReader - to read the xml file
  private $xml_reader = NULL;
  // string - complete path to xml file to load
  private $filepath = '';
  // array - xml content converted to an array
  private $arr_content = array();

  // constructor
  public function __construct($filepath = '') {
    $this->xml_reader = new XMLReader();
    $this->filepath = $filepath;
  }

  /**
   * returns the arr_content property
   * @return $arr_content xml content converted to an array
   */
  public function get_content_array() {
    return $this->arr_content;
  }

  /**
   * opens the xml file
   */
  private function load_xml() {
    if (file_exists($this->filepath)) {
      // open the xml file
      return $this->xml_reader->open($this->filepath);
    }
    else {
      // xml file does not exists
      return FALSE;
    }
  }

  /**
   * parses the xml contents to an array
   * 
   * @param $index the record index to convert
   */
  public function convert_xml_record($index) {
    if (!empty($index) && $index >= 1) {
      // open xml file
      if ($this->load_xml()) {
        // set the pointer to the first contenttype element
        while ($this->xml_reader->read()) {
          if ($this->xml_reader->getAttribute('type') == 'contenttype') {
            break;
          }
        }
        
        $pointer = 1;
  
        // set the pointer to the passed index if possible
        while ($pointer != $index && $this->xml_reader->next()) {
          if ($this->xml_reader->getAttribute('type') == 'contenttype') {
            $pointer++;
          }
        }
  
        if ($pointer == $index) {
          // get the current node as a DOMNode
          $dom = $this->xml_reader->expand();
          // convert the DOMNode into a SimpleXML Object
          $dom_doc = new DOMDocument('1.0', 'utf-8');
          $simple_xml_obj = simplexml_import_dom($dom_doc->importNode($dom, TRUE));
          
          $this->arr_content = array();
          return $this->convertXmlObjToArr($simple_xml_obj, $this->arr_content);
        }
        else {
          return FALSE;
        }
      }
      else {
        return FALSE;
      }
    }
    else {
      return FALSE;
    }
  }
  
  /**
   * get the number of contenttype elements in the xml file
   * 
   * @return number of elements
   */
  public function get_number_of_elements() {
    $number_of_elements = 0;
    
    // open xml file
    $this->load_xml();
    // set the pointer to the first contenttype element
    while ($this->xml_reader->read()) {
      if ($this->xml_reader->getAttribute('type') == 'contenttype') {
        $number_of_elements++;
        break;
      }
    }

    // read the xml to the end and return the count
    while ($this->xml_reader->next()) {
      if ($this->xml_reader->getAttribute('type') == 'contenttype') {
        $number_of_elements++;
      }
    }
    
    return $number_of_elements;
  }

  /**
   * Converts a SimpleXMLElement into an array
   *
   * @author http://www.codingforums.com/showthread.php?t=87283
   *
   * @param $node the SimpleXMLElement object
   * @param $arr the array to store the content
   */
  private function convertXmlObjToArr($node, &$arr) {

    $nextIdx = count($arr);
    
    $arr[$nextIdx] = array();
    $arr[$nextIdx]['@name'] = drupal_strtolower((string) $node->getName());
    $arr[$nextIdx]['@attributes'] = array();
    
    $attributes = $node->attributes();
    
    foreach ($attributes as $attributeName => $attributeValue) {
      $attribName = drupal_strtolower(trim((string) $attributeName));
      $attribVal = trim((string) $attributeValue);
      $arr[$nextIdx]['@attributes'][$attribName] = $attribVal;
    }
    
    $text = (string) $node;
    $text = trim($text);
    
    $entities = Contentparser::entitiesToReplace();
    $text = str_replace($entities['search'], $entities['replace'], $text);
    
    $text = html_entity_decode($text, ENT_COMPAT, 'UTF-8');
    
    if (drupal_strlen($text) > 0) {
      $arr[$nextIdx]['@text'] = $text;
    }
    
    $arr[$nextIdx]['@children'] = array();
    $children = $node->children();
    
    foreach ($children as $child) {
      $this->convertXmlObjToArr($child, $arr[$nextIdx]['@children']);
    }
    
    return TRUE;
  }
  
  /**
   * Returns an array with utf-8 entities and the HTML replacement 
   */
  public static function entitiesToReplace() {
    $entities['search'] = array();
    $entities['replace'] = array();
    
    $entities['search'][] = '&apos;'; // &apos; is not supported from IE
    $entities['replace'][] = '&#39;';
    
    $entities['search'][] = '&#x96;';
    $entities['replace'][] = '&ndash;';
    
    $entities['search'][] = '&#x84;';
    $entities['replace'][] = '&bdquo;';
    
    $entities['search'][] = '&#x93;';
    $entities['replace'][] = '&ldquo;';
    
    $entities['search'][] = '&#x94;';
    $entities['replace'][] = '&rdquo;';
    
    $entities['search'][] = '&#x92;';
    $entities['replace'][] = '&rsquo;';
    
    $entities['search'][] = '&#x91;';
    $entities['replace'][] = '&rsquo;';
    
    $entities['search'][] = '&#x82;';
    $entities['replace'][] = '&sbquo;';
        
    $entities['search'][] = '&#x85;';
    $entities['replace'][] = '&hellip;';
    
    $entities['search'][] = '&#x97;';
    $entities['replace'][] = '&mdash;';
        
    return $entities;
  }

  // destructor
  public function __destruct() {
    // not needed - placeholder
  }
  
}