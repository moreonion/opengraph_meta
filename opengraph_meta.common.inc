<?php

define('OPENGRAPH_META_PERM_ADMIN', 'administer Open Graph meta tags');
define('OPENGRAPH_META_PERM_EDIT', 'edit Open Graph meta tags');

define('OPENGRAPH_META_VAR_CONTENT_TYPES_ENABLED', 'opengraph_meta_types_enabled');


class OpenGraphMeta {
  /** Singleton instance. */
  private static $instance = NULL;

  private $settings_obj = NULL;

 /**
   * Constructor
   */
  private function __construct() {
    $this->settings_obj = new OGMDrupalSettings();
  }

  /** Get singleton instance. */
  public static function instance() {
    if (empty(self::$instance)) {
      self::$instance = new OpenGraphMeta();
    }
    return self::$instance;
  }

  /**
   * Get whether meta tags are enabled for the given content type.
   * @return TRUE if so; FALSE otherwise.
   */
  public function tags_are_enabled_for_content_type($type) {
    $content_types = $this->settings_obj->get(OPENGRAPH_META_VAR_CONTENT_TYPES_ENABLED, array());
    $content_types = array_filter($content_types);
    // if no content types specifically set OR if this content type is set then tags are enabled
    return empty($content_types) || !empty($content_types[$type]);
  }

  /**
   * Harvest all images from the given node.
   *
   * @return array(array('title' => , 'alt' => , 'url' =>))
   */
  public function harvest_images_from_node($node) {
    // extract image fields
    $ret = array();
    OpenGraphMetaDrupalLayer::extract_image_fields((array)$node, $ret);

    // extract all images from body content
    $body = OpenGraphMetaDrupalLayer::get_node_body($node);
    if (!empty($body)) {
      libxml_use_internal_errors(TRUE); // turn off libxml errors for now
      $doc = new DOMDocument();
      $doc->loadHTML($body);
      $list = $doc->getElementsByTagName('img');
      for ($i=0; $list->length > $i; ++$i) {
        $item = $list->item($i);
        if ($item->hasAttribute('src')) {
          $url = $item->getAttribute('src');
          if (!empty($url)) {
            $thumb_url = image_style_url('thumbnail', $url);
            $ret[$url] = array('title' => $url, 'alt' => $url, 'url' => $thumb_url);
          }
        }
      }
      libxml_use_internal_errors(FALSE); // turn libxml errors back on
    }

    return $ret;
  }


  /**
   * Log a warning message.
   * @param $msg the message.
   */
  public function warn($msg) {
    watchdog('opengraph_meta', $msg, array(), WATCHDOG_WARNING);
  }
  

  /**
   * FOR TESTING PURPOSES ONLY!
   * Replace the internally used data and config instances with the given ones.
   */
  public function __set_objects($settings_obj) {
    $this->settings_obj = $settings_obj;
  }

  /**
   * FOR TESTING PURPOSES ONLY!
   * Get the internally used data and config instances with the given ones.
   */
  public function __get_objects() {
    return [$this->settings_obj];
  }
}


/** Interface to getting/setting config settings. */
interface OGMSettings {
  public function get($name, $default);
  public function set($name, $value);
}
/** Implementation which uses Drupal's variables store. */
class OGMDrupalSettings implements OGMSettings {
  public function get($name, $default) {
    return variable_get($name, $default);
  }
  public function set($name, $value) {
    variable_set($name, $value);
  }
}


/**
 * Drupal compatibility layer.
 *
 * Abstracts away differences between Drupal versions.
 */
class OpenGraphMetaDrupalLayer {
  private function __construct() {}


  /** Get all available node content types. */
  public static function get_node_types() {
    return node_type_get_types();
  }

  /**
   * Get contents of node body.
   * @param  $node the node object.
   * @return empty string if no body found.
   */
  public static function get_node_body($node) {
    $body = '';
    $lang = field_language('node', $node, 'body');
    $lang = $lang ? $lang : LANGUAGE_NONE;
    if (!empty($node->body[$lang]['0']['value'])) {
      $body = $node->body[$lang]['0']['value'];
    }
    return $body;
  }


  /**
   * Harvest images from node's image fields.
   *
   * array_walk_recursive() doesn't give us enough flexibility so we do the recursion manually.
   *
   * @param $resultarray will hold results.
   */
  public static function extract_image_fields($fields, array &$resultarray) {
    if (is_array($fields)) {
      if (!empty($fields['filemime']) && FALSE !== stripos($fields['filemime'], 'image') && !empty($fields['uri'])) {
        $url = $fields['uri'];
        $thumb_url = image_style_url('thumbnail', $fields['uri']);

        $resultarray[$url] = array(
          'title' => !empty($fields['title']) ? $fields['title'] : $url,
          'alt' => !empty($fields['alt']) ? $fields['alt'] : $url,
          'url' => $thumb_url,
        );
      }
      else {
        foreach ($fields as $cv) {
          self::extract_image_fields($cv, $resultarray);
        }
      }
    }
  }

  /** Get rendered IMG tag for the OGMT node image selector. */
  public static function theme_selector_image($image) {
    $attributes = array('class' => 'opengraph-thumb');
    $abs_path = url(ltrim($image['url'],'/'));
    return theme('image',
      array(
        'path' => $abs_path,
        'alt' => $image['alt'],
        'height' => '60px',
        'attributes' => array_merge($attributes, array('title' => $image['title']))
      )
    );
  }
}

class OpengraphImageMetaTag extends DrupalTextMetaTag {
  public function getValue(array $options = array()) {
    $value = parent::getValue($options);
    $new_urls = [];
    foreach (explode(',', $value) as $p) {
      $new_urls[] = url(file_create_url(ltrim($p,'/')), array('absolute' => TRUE));
    }
    return implode(',', $new_urls);
  }
}

