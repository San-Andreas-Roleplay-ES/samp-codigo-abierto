-- Run only through test-shooting-practice-sql.ps1 in its disposable database.
SET @sp_checks = 0;
DELIMITER $$
CREATE PROCEDURE sp_test_assert(IN valid BOOLEAN, IN label_text VARCHAR(128))
BEGIN
    IF valid IS NULL OR valid = FALSE THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = label_text;
    END IF;
    SET @sp_checks = @sp_checks + 1;
END$$
CREATE PROCEDURE sp_test_reject(IN statement_text TEXT, IN error_number INT, IN label_text VARCHAR(128))
BEGIN
    DECLARE actual_error INT DEFAULT 0;
    SET @sp_statement = statement_text;
    BEGIN
        DECLARE CONTINUE HANDLER FOR SQLEXCEPTION
            GET DIAGNOSTICS CONDITION 1 actual_error = MYSQL_ERRNO;
        PREPARE sp_check_statement FROM @sp_statement;
        EXECUTE sp_check_statement;
        DEALLOCATE PREPARE sp_check_statement;
    END;
    CALL sp_test_assert(actual_error = error_number, label_text);
END$$
DELIMITER ;

CALL sp_test_assert((SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = DATABASE()) = 16, 'all sixteen tables exist');
CALL sp_test_assert((SELECT COUNT(*) FROM information_schema.columns WHERE table_schema = DATABASE() AND table_name = 'sp_zone_versions' AND column_name IN ('start_x','start_y','start_z','start_heading')) = 0, 'legacy single-origin columns are removed');

INSERT INTO sp_subjects (id,provider,external_key,display_name) VALUES (1,'test','character-1','Personaje de prueba');
INSERT INTO sp_groups (id,provider,kind,external_key,display_name) VALUES (1,'test','faction','faction-1','Facción de prueba');
INSERT INTO sp_zones (id,created_by_subject_id) VALUES (1,1),(2,1);
INSERT INTO sp_zone_versions (id,zone_id,version_number,name,created_by_subject_id)
    VALUES (11,1,1,'Campo principal',1),(12,1,2,'Campo actualizado',1),(22,2,1,'Otra zona',1);
CALL sp_test_assert((SELECT access_mode = 'open' AND history_policy = 'none' AND start_indicators_enabled = 0 FROM sp_zone_versions WHERE id = 11), 'file-compatible defaults: public, unrecorded, indicators off');
CALL sp_test_assert((SELECT COUNT(*) FROM sp_zone_starts WHERE zone_version_id = 12) = 0, 'partial inactive version may have no posts');

INSERT INTO sp_zone_starts (id,zone_version_id,sort_order,name,position_x,position_y,position_z,heading)
    VALUES (111,11,1,'Caseta A',0,0,5.2813,181.2324),(112,11,2,'Caseta B',0,6,5.2813,181.2324),
    (119,11,10,NULL,0,10,5.2813,0),(121,12,1,'Caseta nueva',0,0,5.2813,0),(221,22,1,NULL,200,0,5.2813,0);
CALL sp_test_assert((SELECT COUNT(*) FROM sp_zone_starts WHERE zone_version_id = 11) = 3, 'multiple sparse posts in one version');
CALL sp_test_assert((SELECT heading = 181.2324 AND position_z = 5.2813 AND name = 'Caseta B' FROM sp_zone_starts WHERE id = 112), 'post transforms and names round trip');
CALL sp_test_reject('INSERT INTO sp_zone_starts (zone_version_id,sort_order,position_x,position_y,position_z) VALUES (11,2,1,1,1)',1062,'duplicate post order rejected');
CALL sp_test_reject('INSERT INTO sp_zone_starts (zone_version_id,sort_order,position_x,position_y,position_z) VALUES (11,0,1,1,1)',3819,'post order zero rejected');
CALL sp_test_reject('INSERT INTO sp_zone_starts (zone_version_id,sort_order,position_x,position_y,position_z) VALUES (11,11,1,1,1)',3819,'eleventh post rejected');
CALL sp_test_reject('INSERT INTO sp_zone_starts (zone_version_id,sort_order,position_x,position_y,position_z) VALUES (999,1,1,1,1)',1452,'unknown post version rejected');
CALL sp_test_reject('UPDATE sp_zone_starts SET heading = 360 WHERE id = 112',3819,'noncanonical post heading rejected');
CALL sp_test_reject('UPDATE sp_zone_starts SET position_x = 20001 WHERE id = 112',3819,'out of bounds post coordinate rejected');
CALL sp_test_reject('UPDATE sp_zone_starts SET name = REPEAT(''x'',33) WHERE id = 112',1406,'post name cap enforced');

UPDATE sp_zone_versions SET start_indicators_enabled = 1 WHERE id = 11;
CALL sp_test_assert((SELECT start_indicators_enabled FROM sp_zone_versions WHERE id = 11) = 1 AND (SELECT start_indicators_enabled FROM sp_zone_versions WHERE id = 22) = 0,'indicator preference is independent by version');
CALL sp_test_reject('UPDATE sp_zone_versions SET start_indicators_enabled = 2 WHERE id = 11',3819,'invalid indicator flag rejected');
CALL sp_test_reject('UPDATE sp_zone_versions SET start_radius = 2 WHERE id = 11',3819,'start radius stays at the current engine constant');
CALL sp_test_reject('UPDATE sp_zone_versions SET movement_radius = 4 WHERE id = 11',3819,'departure radius stays at the current engine constant');
CALL sp_test_reject('UPDATE sp_zone_versions SET interior_id = 256 WHERE id = 11',3819,'interior cap matches runtime');
CALL sp_test_reject('UPDATE sp_zone_versions SET virtual_world_id = 2147483648 WHERE id = 11',3819,'world IDs fit Pawn');
CALL sp_test_reject('UPDATE sp_zone_versions SET name = REPEAT(''x'',49) WHERE id = 11',1406,'zone name cap enforced');
CALL sp_test_reject('UPDATE sp_zone_versions SET name = ''   '' WHERE id = 11',3819,'empty zone name rejected');
UPDATE sp_zone_versions SET target_exposure_ms = 100, target_interval_ms = 0, preparation_ms = 1000 WHERE id = 12;
CALL sp_test_assert((SELECT target_exposure_ms = 100 AND target_interval_ms = 0 AND preparation_ms = 1000 FROM sp_zone_versions WHERE id = 12),'minimum exposure and zero interval accepted');
CALL sp_test_reject('UPDATE sp_zone_versions SET target_exposure_ms = 50 WHERE id = 12',3819,'exposure below 0.1 seconds rejected');
CALL sp_test_reject('UPDATE sp_zone_versions SET target_exposure_ms = 150 WHERE id = 12',3819,'non-tenth exposure rejected');
CALL sp_test_reject('UPDATE sp_zone_versions SET target_interval_ms = 30001 WHERE id = 12',3819,'interval bounds enforced');
CALL sp_test_reject('UPDATE sp_zone_versions SET preparation_ms = 1500 WHERE id = 12',3819,'fractional preparation rejected');
CALL sp_test_reject('UPDATE sp_zone_versions SET minimum_net_score = 0 WHERE id = 12',3819,'zero required score rejected');
CALL sp_test_reject('UPDATE sp_zone_versions SET maximum_identification_errors = 11 WHERE id = 12',3819,'error cap enforced');

INSERT INTO sp_zone_weapons (zone_version_id,weapon_id) VALUES (11,24),(11,31);
CALL sp_test_assert((SELECT COUNT(*) FROM sp_zone_weapons WHERE zone_version_id = 11) = 2,'multiple permitted weapons');
CALL sp_test_reject('INSERT INTO sp_zone_weapons VALUES (11,21)',3819,'weapon below supported range rejected');
CALL sp_test_reject('INSERT INTO sp_zone_weapons VALUES (11,35)',3819,'weapon above supported range rejected');
CALL sp_test_reject('INSERT INTO sp_zone_weapons VALUES (11,24)',1062,'duplicate permitted weapon rejected');
INSERT INTO sp_zone_targets (id,zone_version_id,sort_order,name,model_id,behavior,position_x,position_y,position_z) VALUES
    (311,11,1,'Blanco 1',1271,'shoot',8,0,5.2813),
    (312,11,2,'Blanco 2',1583,'protected',16,0,5.2813),
    (313,11,3,'Blanco 3',1584,'shoot',24,0,5.2813),
    (314,11,4,'Blanco 4',1585,'protected',32,0,5.2813),
    (315,11,5,'Blanco 5',1586,'shoot',40,0,5.2813),
    (316,11,6,'Blanco 6',1543,'protected',48,0,5.2813),
    (317,11,7,'Blanco 7',1544,'shoot',56,0,5.2813),
    (318,11,8,'Blanco 8',2601,'protected',64,0,5.2813),
    (319,11,9,'Blanco 9',1238,'shoot',72,0,5.2813);

CALL sp_test_assert((SELECT COUNT(*) FROM sp_zone_targets WHERE zone_version_id = 11) = 9,'all current catalogue models persist');
CALL sp_test_reject('UPDATE sp_zone_targets SET sort_order = 11 WHERE id = 311',3819,'target cap matches runtime');
CALL sp_test_reject('UPDATE sp_zone_targets SET name = REPEAT(''x'',33) WHERE id = 311',1406,'target name cap enforced');
CALL sp_test_reject('UPDATE sp_zone_targets SET position_z = -20001 WHERE id = 311',3819,'target coordinate bounds enforced');
CALL sp_test_reject('UPDATE sp_zone_targets SET behavior = ''bad'' WHERE id = 311',3819,'invalid target behavior rejected');
UPDATE sp_zone_targets SET model_id = -1000 WHERE id = 311;
CALL sp_test_assert((SELECT model_id = -1000 FROM sp_zone_targets WHERE id = 311),'signed custom model remains representable in the proposal');
UPDATE sp_zone_targets SET model_id = 1271 WHERE id = 311;
CALL sp_test_reject('UPDATE sp_zone_targets SET model_id = 0 WHERE id = 311',3819,'zero model rejected');
CALL sp_test_assert((SELECT MIN(SQRT(POW(t.position_x-s.position_x,2)+POW(t.position_y-s.position_y,2)+POW(t.position_z-s.position_z,2))) >= 3 AND MAX(SQRT(POW(t.position_x-s.position_x,2)+POW(t.position_y-s.position_y,2)+POW(t.position_z-s.position_z,2))) <= 100 FROM sp_zone_targets t JOIN sp_zone_starts s ON s.zone_version_id = t.zone_version_id WHERE t.zone_version_id = 11),'fixture validates target distances from every post');

UPDATE sp_zone_versions SET history_policy = 'evaluations_only' WHERE id = 11;
UPDATE sp_zones SET current_version_id = 11, is_enabled = 1 WHERE id = 1;
CALL sp_test_reject('UPDATE sp_zones SET current_version_id = 22 WHERE id = 1',1452,'current version cannot belong to another zone');
INSERT INTO sp_attempts (id,attempt_key,zone_version_id,start_point_id,participant_subject_id,participant_name_at_start,started_at_utc,scoring_version)
    VALUES (1,'00000000-0000-0000-0000-000000000001',11,112,1,'Personaje de prueba','2026-10-11 12:00:00','shooting-practice-v1');
CALL sp_test_assert((SELECT a.start_point_id = 112 AND s.name = 'Caseta B' FROM sp_attempts a JOIN sp_zone_starts s ON s.id = a.start_point_id AND s.zone_version_id = a.zone_version_id WHERE a.id = 1),'evaluation references chosen booth within the exact version');
CALL sp_test_reject('INSERT INTO sp_attempts (attempt_key,zone_version_id,start_point_id,participant_subject_id,participant_name_at_start,started_at_utc,scoring_version) VALUES (''cross-zone'',11,221,1,''X'',''2026-10-11 12:00:00'',''v1'')',1452,'start from another zone rejected');
CALL sp_test_reject('INSERT INTO sp_attempts (attempt_key,zone_version_id,start_point_id,participant_subject_id,participant_name_at_start,started_at_utc,scoring_version) VALUES (''cross-version'',11,121,1,''X'',''2026-10-11 12:00:00'',''v1'')',1452,'start from another version of same zone rejected');
CALL sp_test_reject('INSERT INTO sp_attempts (attempt_key,zone_version_id,start_point_id,participant_subject_id,participant_name_at_start,started_at_utc,scoring_version) VALUES (''missing-start'',11,999,1,''X'',''2026-10-11 12:00:00'',''v1'')',1452,'unknown chosen post rejected');
CALL sp_test_reject('INSERT INTO sp_attempts (attempt_key,zone_version_id,participant_subject_id,participant_name_at_start,started_at_utc,scoring_version) VALUES (''null-start'',11,1,''X'',''2026-10-11 12:00:00'',''v1'')',1364,'chosen post is mandatory for evaluations');
CALL sp_test_reject('DELETE FROM sp_zone_starts WHERE id = 112',1451,'historical chosen booth cannot be deleted');
CALL sp_test_reject('UPDATE sp_attempts SET passed = 1 WHERE id = 1',3819,'in-progress evaluation cannot have a verdict');
UPDATE sp_attempts SET status = 'interrupted', ended_at_utc = '2026-10-11 12:00:03', interruption_reason = 'left_post', interruption_message = 'Cancelada automáticamente por salir del puesto de tiro.', shots_fired = 1, identification_error_count = 1, net_score = -1 WHERE id = 1;
CALL sp_test_assert((SELECT net_score = -1 AND passed IS NULL FROM sp_attempts WHERE id = 1),'interruption retains negative score and no verdict');
CALL sp_test_assert((SELECT interruption_message = 'Cancelada automáticamente por salir del puesto de tiro.' FROM sp_attempts WHERE id = 1),'localized interruption message is retained separately from its stable code');
CALL sp_test_reject('UPDATE sp_attempts SET passed = 0 WHERE id = 1',3819,'interrupted evaluation cannot have a failed verdict');
UPDATE sp_zones SET is_enabled = 0, current_version_id = 12 WHERE id = 1;
CALL sp_test_assert((SELECT a.zone_version_id = 11 AND a.start_point_id = 112 AND s.name = 'Caseta B' FROM sp_attempts a JOIN sp_zone_starts s ON s.id = a.start_point_id WHERE a.id = 1),'switching current version preserves original booth history');
UPDATE sp_zones SET archived_at_utc = '2026-10-11 13:00:00' WHERE id = 1;
CALL sp_test_assert((SELECT COUNT(*) FROM sp_attempts WHERE id = 1) = 1,'archiving preserves evaluation history');
CALL sp_test_reject('UPDATE sp_zones SET is_enabled = 1 WHERE id = 1',3819,'archived zone cannot be enabled');

SELECT CONCAT('[Practice SQL tests] ',@sp_checks,' assertions');
SELECT '[Practice SQL tests] PASS';
