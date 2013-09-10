<?php
/**
 * @file
 * islandora-basic-collection.tpl.php
 */
$object = $variables['islandora_object'];
$members = $variables['members'];
$critical_edition = array_search('islandora:criticalEditionCModel', $members);
$edition_object = islandora_object_load($critical_edition);
$transcriptions = array_keys($members, 'islandora:transcriptionCModel');
drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
drupal_set_title($object->label);
?>
<div class ="versionable_object_header">
  <?php print "<h1>" . $object->label . "</h12>"; ?>
</div>
<div class="islandora_cwrcwriter_object">
  <?php if ($critical_edition): ?>
    <?php print l($edition_object->label, "islandora/object/${critical_edition}");  ?>
  <?php endif; ?>
</div>

<div class ="islandora_critical_transcriptions" >
  <?php foreach ($transcriptions as $transcription): ?>
    <?php 
    $transcription_object = islandora_object_load($transcription);
    print l($transcription_object->label, "islandora/object/$transcription_object");
    print "<br />";
    ?>
  <?php endforeach; ?>
</div>
