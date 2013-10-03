<?php

/**
 * @file
 * Holds utility functions for islandora_critical_editions.
 */

/**
 * Returns PID and title of all critical editions associated with an object.
 *
 * @param AbstractObject $fedora_object
 *   The base object to check for related critical editions.
 *
 * @return array
 *   The list of critical editions.
 */
function islandora_critical_edition_get_critical_editions(AbstractObject $fedora_object) {
  $edition_pids = array();

  $query = "
select \$subject \$title from <#ri>
where (\$subject <" . ISLANDORA_RELS_EXT_URI . ISLANDORA_CRITICAL_EDITION_IS_CRITICAL_EDITION_OF . ">'" . $fedora_object->id . "'
and \$subject <" . FEDORA_MODEL_URI . "state> <" . FEDORA_MODEL_URI . "Active>
and \$subject <dc:title> \$title)
";

  $objects = $fedora_object->repository->ri->itqlQuery($query, 'unlimited');
  if (!empty($objects)) {
    foreach ($objects as $object) {
      $edition_pids[$object['subject']['value']] = $object['title']['value'];
    }
  }

  return $edition_pids;
}

/**
 * Returns the base object associated with a critical edition.
 *
 * @param AbstractObject $fedora_object
 *   The critical edition object to check for a related base object.
 *
 * @return string
 *   The PID of the base object.
 */
function islandora_critical_edition_get_object_of_critical_edition(AbstractObject $fedora_object) {
  $critical_edition_relationships = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, ISLANDORA_CRITICAL_EDITION_IS_CRITICAL_EDITION_OF);
  return $critical_edition_relationships[0]['object']['value'];
}

/**
 * Constructs CWRC datastream on an object if none exists.
 *
 * @param AbstractObject $fedora_object
 *   The object to create the CWRC datastream for. It should have an OCR
 *   datastream.
 */
function islandora_critical_edition_add_cwrc_datastream(AbstractObject $fedora_object) {
  $ocr_in = $fedora_object['OCR']->content;
  $lines = explode("\n", $ocr_in);
  $preface = '<?xml version="1.0" encoding="UTF-8"?>
        <TEI xml:id="struct_35" xmlns="http://www.tei-c.org/ns/1.0">
            <span _tag="text" _struct="true" id="struct_02" _display="text">';

  $postscript = '            </span>
        </TEI>';
  $ocr = "";
  if (is_array($lines)) {
    foreach ($lines as $line) {
      $line = trim(preg_replace('/&£/', '£', $line));
      $line = htmlspecialchars($line);
      $ocr .= $line . '<br />';
    }
  }
  if (empty($ocr)) {
    $ocr = "Blank Page";
  }

  $cwrc_stream = $preface . $ocr . $postscript;
  $cwrc_datastream = $fedora_object->constructDatastream('CWRC');
  $cwrc_datastream->label = 'CWRC';
  $cwrc_datastream->mimetype = 'text/plain';
  $cwrc_datastream->setContentFromString($cwrc_stream);
  $fedora_object->ingestDatastream($cwrc_datastream);
}

/**
 * Returns associated objects.
 *
 * @param String $object_pid
 *   PID of object
 * @param String $content_model
 *   Optional Content Model to restrict results.
 *
 * @return array
 *   associative array of PID => Content Model pairs.
 */
function islandora_critical_edition_get_members($object_pid, $content_model = NULL) {
  $qualifier = $content_model ? "fm:hasModel <info:fedora/$content_model>;" : "";
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fr: <" . FEDORA_RELS_EXT_URI . ">
            SELECT ?subject ?object
            FROM <#ri>
            WHERE {
              {?subject fm:state fm:Active;
                        fr:isMemberOfCollection <info:fedora/$object_pid>;
                        fm:hasModel ?object;
                        $qualifier
                          
              }
            UNION 
              {?subject fm:state fm:Active;
                        fr:isMemberOf <info:fedora/$object_pid>;
                        fm:hasModel ?object;
                        $qualifier
              }
            }
            ORDER BY ?subject";
  $objects = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  foreach ($results as $result) {
    if ($result['object']['value'] != 'fedora-system:FedoraObject-3.0') {
      $objects[$result['subject']['value']] = $result['object']['value'];
    }
  }
  return $objects;
}

/**
 * Gets all transcriptions related to this object.
 *
 * @param String $versionable_object_pid
 *   PID of object
 *
 * @return array
 *   Array of pid=>label pairs
 */
function islandora_critical_get_related_transcriptions($versionable_object_pid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = "
            PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fr: <" . FEDORA_RELS_EXT_URI . ">

            SELECT DISTINCT ?subject ?label FROM <#ri>
                WHERE {
                        {?subject fm:hasModel <info:fedora/islandora:transcriptionCModel>;
                                  fr:isMemberOf <info:fedora/$versionable_object_pid>;
                       }
                 OPTIONAL{
                           ?subject fm:label  ?label; 
                          }
                       }";
  $objects = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  foreach ($results as $result) {
    $objects[$result['subject']['value']] = $result['label']['value'];
  }
  return $objects;
}

/**
 * Returns array of possible apparatus datastreams.
 *
 * @return array
 *   Associative array of dsid => label
 */
function islandora_critical_apparatus_get_datastreams() {
  return array(
    'title_page' => t('Title Page'),
    'epigraph' => t('Epigraph'),
    'preface' => t('Preface'),
    'foreword' => t('Foreword'),
    'introduction' => t('Introduction'),
    'illustrations' => t('List of Illustrations, Figures, Graphs'),
    'acknowledgements' => t('Acknowledgements'),
    'editorial_introduction' => t('Editorial Introduction'),
    'headnote' => t('Headnote'),
    'footnote' => t('Footnotes'),
    'endnotes' => t('Endnotes'),
    'epilogue' => t('Epilogue'),
    'afterword' => t('Afterword'),
    'editorial_procedure' => t('Editorial Procedure'),
  );
}

/**
 * Returns all specified objects in the Fedora Repository.
 *
 * @return array
 *   array of PID => Label pairs
 */
function islandora_critical_edition_get_related_objects($content_model, $offset = 0, $limit = 'unlimited') {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = islandora_critical_edition_get_associated_objects_query($content_model, $offset);
  $objects = array();
  $results = $tuque->repository->ri->sparqlQuery($query, $limit);
  foreach ($results as $result) {
    $objects[$result['subject']['value']] = $result['label']['value'];
  }
  return $objects;
}

/**
 * Builds and returns query to get all book objects in repository.
 * @param type $offset
 * @return type
 */
function islandora_critical_edition_get_associated_objects_query($content_model, $offset) {
  $query = "PREFIX fm: <info:fedora/fedora-system:def/model#>
                SELECT DISTINCT ?subject ?label FROM <#ri>
                WHERE {
                        {?subject fm:hasModel <info:fedora/$content_model>;
                       }
                 OPTIONAL{
                           ?subject fm:label  ?label; 
                          }
                       }
            ORDER BY ?label OFFSET $offset";
  return $query;
}