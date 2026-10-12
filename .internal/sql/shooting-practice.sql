-- DESIGN PROPOSAL, 2026-10-11. Not a production migration or an enabled backend.
-- Reference dialect: MySQL 8.0.16+, InnoDB, enforced CHECK constraints.
-- Apply only to an empty disposable database when validating this proposal.
-- The host gamemode owns authentication, group membership and administrative roles.
-- See ../shooting-practice-data-model.md for publication and authorization rules.

SET NAMES utf8mb4;
SET time_zone = '+00:00';

-- 1. Identity references. These tables do not implement login or membership.
CREATE TABLE sp_subjects (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    provider VARCHAR(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    external_key VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    display_name VARCHAR(100) NOT NULL,
    created_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    disabled_at_utc DATETIME(3) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_sp_subject_source (provider, external_key)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE sp_groups (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    provider VARCHAR(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    kind VARCHAR(8) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    external_key VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    display_name VARCHAR(100) NOT NULL,
    created_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    disabled_at_utc DATETIME(3) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_sp_group_source (provider, kind, external_key),
    CONSTRAINT ck_sp_group_kind CHECK (kind IN ('group', 'faction'))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Rank references belong to one host group. Numeric IDs do not encode seniority.
-- Membership, current ranks and hierarchy comparisons remain host responsibilities.
CREATE TABLE sp_group_ranks (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    group_id BIGINT UNSIGNED NOT NULL,
    external_key VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    display_name VARCHAR(100) NOT NULL,
    created_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    disabled_at_utc DATETIME(3) NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_sp_rank_source (group_id, external_key),
    UNIQUE KEY uq_sp_rank_group (group_id, id),
    CONSTRAINT fk_sp_rank_group FOREIGN KEY (group_id)
        REFERENCES sp_groups (id) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 2. Stable zones and immutable configuration versions.
-- A new zone has no current version and remains disabled until configured.
CREATE TABLE sp_zones (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    current_version_id BIGINT UNSIGNED NULL,
    is_enabled TINYINT NOT NULL DEFAULT 0,
    created_by_subject_id BIGINT UNSIGNED NOT NULL,
    created_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    archived_at_utc DATETIME(3) NULL,
    PRIMARY KEY (id),
    KEY ix_sp_zone_current (id, current_version_id),
    KEY ix_sp_zone_listing (archived_at_utc, is_enabled, id),
    CONSTRAINT fk_sp_zone_creator FOREIGN KEY (created_by_subject_id)
        REFERENCES sp_subjects (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_zone_enabled CHECK (is_enabled IN (0, 1)),
    CONSTRAINT ck_sp_zone_ready CHECK (is_enabled = 0 OR current_version_id IS NOT NULL),
    CONSTRAINT ck_sp_zone_archived CHECK (archived_at_utc IS NULL OR is_enabled = 0)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE sp_zone_versions (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    zone_id BIGINT UNSIGNED NOT NULL,
    version_number INT UNSIGNED NOT NULL,
    name VARCHAR(48) NOT NULL,
    access_mode VARCHAR(12) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT 'open',
    -- Independent of access: training is unrecorded; evaluations require this policy.
    history_policy VARCHAR(16) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT 'none',
    -- One zone-level preference applies to every start. Model/type/height remain code constants.
    start_indicators_enabled TINYINT NOT NULL DEFAULT 0,
    interior_id SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    virtual_world_id INT UNSIGNED NOT NULL DEFAULT 0,
    start_radius DECIMAL(8,3) NOT NULL DEFAULT 1.5,
    movement_radius DECIMAL(8,3) NOT NULL DEFAULT 3.0,
    preparation_ms INT UNSIGNED NOT NULL DEFAULT 3000,
    target_exposure_ms INT UNSIGNED NOT NULL DEFAULT 5000,
    target_interval_ms INT UNSIGNED NOT NULL DEFAULT 1000,
    minimum_net_score INT NOT NULL DEFAULT 1,
    maximum_identification_errors INT UNSIGNED NOT NULL DEFAULT 0,
    order_mode VARCHAR(8) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT 'fixed',
    distribution_mode VARCHAR(8) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT 'fixed',
    created_by_subject_id BIGINT UNSIGNED NOT NULL,
    created_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    PRIMARY KEY (id),
    UNIQUE KEY uq_sp_zone_version_number (zone_id, version_number),
    UNIQUE KEY uq_sp_zone_version_owner (zone_id, id),
    CONSTRAINT fk_sp_version_zone FOREIGN KEY (zone_id)
        REFERENCES sp_zones (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_version_creator FOREIGN KEY (created_by_subject_id)
        REFERENCES sp_subjects (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_version_number CHECK (version_number > 0),
    CONSTRAINT ck_sp_version_access CHECK (access_mode IN ('open', 'restricted')),
    CONSTRAINT ck_sp_version_history CHECK (history_policy IN ('none', 'evaluations_only')),
    CONSTRAINT ck_sp_version_name CHECK (CHAR_LENGTH(TRIM(name)) > 0),
    CONSTRAINT ck_sp_version_indicators CHECK (start_indicators_enabled IN (0, 1)),
    CONSTRAINT ck_sp_version_dimension CHECK (interior_id <= 255 AND virtual_world_id <= 2147483647),
    -- Radii are frozen engine constants in this MVP, not per-zone menu settings.
    CONSTRAINT ck_sp_version_radii CHECK (start_radius = 1.5 AND movement_radius = 3.0),
    CONSTRAINT ck_sp_version_timing CHECK (
        preparation_ms BETWEEN 1000 AND 30000 AND MOD(preparation_ms, 1000) = 0
        AND target_exposure_ms BETWEEN 100 AND 60000 AND MOD(target_exposure_ms, 100) = 0
        AND target_interval_ms BETWEEN 0 AND 30000 AND MOD(target_interval_ms, 100) = 0
    ),
    CONSTRAINT ck_sp_version_score CHECK (
        minimum_net_score BETWEEN 1 AND 10 AND maximum_identification_errors BETWEEN 0 AND 10
    ),
    CONSTRAINT ck_sp_version_order CHECK (order_mode IN ('fixed', 'random')),
    CONSTRAINT ck_sp_version_distribution CHECK (distribution_mode IN ('fixed', 'random'))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- The composite FK prevents a zone from selecting another zone's version.
ALTER TABLE sp_zones ADD CONSTRAINT fk_sp_zone_current
    FOREIGN KEY (id, current_version_id) REFERENCES sp_zone_versions (zone_id, id)
    ON DELETE RESTRICT ON UPDATE RESTRICT;

-- Starts are immutable version children. Sparse order 1..10 maps through PracticeStartSlot,
-- never through a persistent Pawn array slot. All starts/targets share the version dimension.
-- A partial inactive version may have no starts; activation validates layout/geometry in the service.
CREATE TABLE sp_zone_starts (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    zone_version_id BIGINT UNSIGNED NOT NULL,
    sort_order TINYINT UNSIGNED NOT NULL,
    name VARCHAR(32) NULL,
    position_x DECIMAL(12,6) NOT NULL,
    position_y DECIMAL(12,6) NOT NULL,
    position_z DECIMAL(12,6) NOT NULL,
    heading DECIMAL(10,6) NOT NULL DEFAULT 0,
    PRIMARY KEY (id),
    UNIQUE KEY uq_sp_start_order (zone_version_id, sort_order),
    UNIQUE KEY uq_sp_start_version (zone_version_id, id),
    CONSTRAINT fk_sp_start_version FOREIGN KEY (zone_version_id)
        REFERENCES sp_zone_versions (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_start_order CHECK (sort_order BETWEEN 1 AND 10),
    CONSTRAINT ck_sp_start_position CHECK (
        position_x BETWEEN -20000 AND 20000
        AND position_y BETWEEN -20000 AND 20000
        AND position_z BETWEEN -20000 AND 20000
    ),
    CONSTRAINT ck_sp_start_heading CHECK (heading >= 0 AND heading < 360)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Targets are rows; no fixed target1..target10 columns or runtime object handles.
CREATE TABLE sp_zone_targets (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    zone_version_id BIGINT UNSIGNED NOT NULL,
    sort_order INT UNSIGNED NOT NULL,
    name VARCHAR(32) NULL,
    -- Signed IDs also allow future SA-MP 0.3.DL custom object models.
    model_id INT NOT NULL,
    behavior VARCHAR(12) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    position_x DECIMAL(12,6) NOT NULL,
    position_y DECIMAL(12,6) NOT NULL,
    position_z DECIMAL(12,6) NOT NULL,
    rotation_x DECIMAL(10,6) NOT NULL DEFAULT 0,
    rotation_y DECIMAL(10,6) NOT NULL DEFAULT 0,
    rotation_z DECIMAL(10,6) NOT NULL DEFAULT 0,
    PRIMARY KEY (id),
    UNIQUE KEY uq_sp_target_order (zone_version_id, sort_order),
    UNIQUE KEY uq_sp_target_version (zone_version_id, id),
    CONSTRAINT fk_sp_target_version FOREIGN KEY (zone_version_id)
        REFERENCES sp_zone_versions (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_target_order CHECK (sort_order BETWEEN 1 AND 10),
    CONSTRAINT ck_sp_target_position CHECK (
        position_x BETWEEN -20000 AND 20000
        AND position_y BETWEEN -20000 AND 20000
        AND position_z BETWEEN -20000 AND 20000
    ),
    CONSTRAINT ck_sp_target_model CHECK (model_id <> 0),
    CONSTRAINT ck_sp_target_behavior CHECK (behavior IN ('shoot', 'protected')),
    CONSTRAINT ck_sp_target_rotation CHECK (
        rotation_x >= 0 AND rotation_x < 360
        AND rotation_y >= 0 AND rotation_y < 360
        AND rotation_z >= 0 AND rotation_z < 360
    )
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE sp_zone_weapons (
    zone_version_id BIGINT UNSIGNED NOT NULL,
    weapon_id SMALLINT UNSIGNED NOT NULL,
    PRIMARY KEY (zone_version_id, weapon_id),
    CONSTRAINT fk_sp_weapon_version FOREIGN KEY (zone_version_id)
        REFERENCES sp_zone_versions (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_weapon_id CHECK (weapon_id BETWEEN 22 AND 34)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Group/faction whitelist only. There is no per-user practice grant in this scope.
CREATE TABLE sp_zone_groups (
    zone_version_id BIGINT UNSIGNED NOT NULL,
    group_id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (zone_version_id, group_id),
    KEY ix_sp_zone_group_lookup (group_id, zone_version_id),
    CONSTRAINT fk_sp_whitelist_version FOREIGN KEY (zone_version_id)
        REFERENCES sp_zone_versions (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_whitelist_group FOREIGN KEY (group_id)
        REFERENCES sp_groups (id) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 3. History foundation. No historical UI or database writes are enabled by this DDL.
-- Only recorded evaluations create attempts; unrecorded training creates no history rows.
-- The service must validate history_policy before registering an evaluation. A FK alone
-- does not enforce eligibility. Changing the current policy never erases old attempts.
-- The selected start is from the same immutable version, preserving booth/origin history.
-- Counters and automatic evaluation are the recorded final summary. Accuracy is derived.
CREATE TABLE sp_attempts (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    attempt_key CHAR(36) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    zone_version_id BIGINT UNSIGNED NOT NULL,
    start_point_id BIGINT UNSIGNED NOT NULL,
    participant_subject_id BIGINT UNSIGNED NOT NULL,
    participant_name_at_start VARCHAR(100) NOT NULL,
    status VARCHAR(12) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT 'in_progress',
    started_at_utc DATETIME(3) NOT NULL,
    ended_at_utc DATETIME(3) NULL,
    interruption_reason VARCHAR(40) CHARACTER SET ascii COLLATE ascii_bin NULL,
    -- Preserve the localized explanation separately from the stable machine-readable code.
    interruption_message VARCHAR(144) NULL,
    scoring_version VARCHAR(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    shots_fired INT UNSIGNED NOT NULL DEFAULT 0,
    hit_count INT UNSIGNED NOT NULL DEFAULT 0,
    shoot_timeout_count INT UNSIGNED NOT NULL DEFAULT 0,
    protected_respected_count INT UNSIGNED NOT NULL DEFAULT 0,
    identification_error_count INT UNSIGNED NOT NULL DEFAULT 0,
    net_score INT NULL,
    passed TINYINT NULL,
    PRIMARY KEY (id),
    UNIQUE KEY uq_sp_attempt_key (attempt_key),
    UNIQUE KEY uq_sp_attempt_version (id, zone_version_id),
    KEY ix_sp_attempt_start (zone_version_id, start_point_id),
    KEY ix_sp_attempt_participant (participant_subject_id, started_at_utc, id),
    KEY ix_sp_attempt_history (zone_version_id, started_at_utc, id),
    KEY ix_sp_attempt_recovery (status, started_at_utc, id),
    CONSTRAINT fk_sp_attempt_version FOREIGN KEY (zone_version_id)
        REFERENCES sp_zone_versions (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_attempt_start FOREIGN KEY (zone_version_id, start_point_id)
        REFERENCES sp_zone_starts (zone_version_id, id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_attempt_participant FOREIGN KEY (participant_subject_id)
        REFERENCES sp_subjects (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_attempt_passed CHECK (passed IS NULL OR passed IN (0, 1)),
    CONSTRAINT ck_sp_attempt_dates CHECK (ended_at_utc IS NULL OR ended_at_utc >= started_at_utc),
    CONSTRAINT ck_sp_attempt_counts CHECK (hit_count + identification_error_count <= shots_fired),
    CONSTRAINT ck_sp_attempt_lifecycle CHECK (
        (status = 'in_progress' AND ended_at_utc IS NULL AND interruption_reason IS NULL
            AND net_score IS NULL AND passed IS NULL)
        OR (status = 'completed' AND ended_at_utc IS NOT NULL AND interruption_reason IS NULL
            AND net_score IS NOT NULL AND passed IS NOT NULL)
        OR (status = 'interrupted' AND ended_at_utc IS NOT NULL AND interruption_reason IS NOT NULL
            AND net_score IS NOT NULL AND passed IS NULL)
    )
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Preserve every matched whitelist group at admission, not live membership history.
CREATE TABLE sp_attempt_groups (
    attempt_id BIGINT UNSIGNED NOT NULL,
    zone_version_id BIGINT UNSIGNED NOT NULL,
    group_id BIGINT UNSIGNED NOT NULL,
    group_name_at_start VARCHAR(100) NOT NULL,
    PRIMARY KEY (attempt_id, group_id),
    KEY ix_sp_attempt_group_history (group_id, attempt_id),
    KEY ix_sp_attempt_group_version (attempt_id, zone_version_id),
    KEY ix_sp_attempt_group_whitelist (zone_version_id, group_id),
    CONSTRAINT fk_sp_attempt_group_owner FOREIGN KEY (attempt_id, zone_version_id)
        REFERENCES sp_attempts (id, zone_version_id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_attempt_group_grant FOREIGN KEY (zone_version_id, group_id)
        REFERENCES sp_zone_groups (zone_version_id, group_id) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Recorded evaluation detail: position and stimulus can differ after random distribution.
-- zone_version_id is deliberate redundancy for composite referential integrity.
CREATE TABLE sp_attempt_targets (
    attempt_id BIGINT UNSIGNED NOT NULL,
    zone_version_id BIGINT UNSIGNED NOT NULL,
    sequence_number INT UNSIGNED NOT NULL,
    position_target_id BIGINT UNSIGNED NOT NULL,
    stimulus_target_id BIGINT UNSIGNED NOT NULL,
    presented_at_utc DATETIME(3) NULL,
    resolved_at_utc DATETIME(3) NULL,
    resolution VARCHAR(24) CHARACTER SET ascii COLLATE ascii_bin NULL,
    shots_fired INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (attempt_id, sequence_number),
    KEY ix_sp_presentation_attempt_version (attempt_id, zone_version_id),
    KEY ix_sp_presentation_position (zone_version_id, position_target_id),
    KEY ix_sp_presentation_stimulus (zone_version_id, stimulus_target_id),
    CONSTRAINT fk_sp_presentation_attempt FOREIGN KEY (attempt_id, zone_version_id)
        REFERENCES sp_attempts (id, zone_version_id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_presentation_position FOREIGN KEY (zone_version_id, position_target_id)
        REFERENCES sp_zone_targets (zone_version_id, id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_presentation_stimulus FOREIGN KEY (zone_version_id, stimulus_target_id)
        REFERENCES sp_zone_targets (zone_version_id, id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_presentation_sequence CHECK (sequence_number > 0),
    CONSTRAINT ck_sp_presentation_dates CHECK (
        resolved_at_utc IS NULL OR (presented_at_utc IS NOT NULL AND resolved_at_utc >= presented_at_utc)
    ),
    CONSTRAINT ck_sp_presentation_resolution CHECK (
        (resolution IS NULL AND resolved_at_utc IS NULL)
        OR (resolution IS NOT NULL AND resolution IN
            ('hit', 'shoot_timeout', 'protected_respected', 'identification_error', 'interrupted')
            AND presented_at_utc IS NOT NULL AND resolved_at_utc IS NOT NULL)
    )
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 4. Future supervision. These grants never authorize configuration or participation.
-- Individual assignments explicitly grant zone-wide read access until revoked.
CREATE TABLE sp_zone_supervisors (
    zone_id BIGINT UNSIGNED NOT NULL,
    subject_id BIGINT UNSIGNED NOT NULL,
    can_review TINYINT NOT NULL DEFAULT 0,
    granted_by_subject_id BIGINT UNSIGNED NOT NULL,
    granted_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    revoked_at_utc DATETIME(3) NULL,
    PRIMARY KEY (zone_id, subject_id),
    KEY ix_sp_supervisor_scope (subject_id, revoked_at_utc, zone_id),
    CONSTRAINT fk_sp_supervisor_zone FOREIGN KEY (zone_id)
        REFERENCES sp_zones (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_supervisor_subject FOREIGN KEY (subject_id)
        REFERENCES sp_subjects (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_supervisor_granter FOREIGN KEY (granted_by_subject_id)
        REFERENCES sp_subjects (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_supervisor_review CHECK (can_review IN (0, 1)),
    CONSTRAINT ck_sp_supervisor_dates CHECK (revoked_at_utc IS NULL OR revoked_at_utc >= granted_at_utc)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- One current rule per zone/group. Read scope is historical admission through that group.
-- Authorization uses current host membership/rank; a whitelist entry is not a read grant.
CREATE TABLE sp_zone_supervisor_rules (
    zone_id BIGINT UNSIGNED NOT NULL,
    group_id BIGINT UNSIGNED NOT NULL,
    rank_mode VARCHAR(8) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    minimum_rank_id BIGINT UNSIGNED NULL,
    can_review TINYINT NOT NULL DEFAULT 0,
    granted_by_subject_id BIGINT UNSIGNED NOT NULL,
    granted_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    revoked_at_utc DATETIME(3) NULL,
    PRIMARY KEY (zone_id, group_id),
    UNIQUE KEY uq_sp_supervisor_rule_mode (zone_id, group_id, rank_mode),
    KEY ix_sp_supervisor_rule_group (group_id, revoked_at_utc, zone_id),
    KEY ix_sp_supervisor_rule_minimum (group_id, minimum_rank_id),
    CONSTRAINT fk_sp_supervisor_rule_zone FOREIGN KEY (zone_id)
        REFERENCES sp_zones (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_supervisor_rule_group FOREIGN KEY (group_id)
        REFERENCES sp_groups (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_supervisor_rule_minimum FOREIGN KEY (group_id, minimum_rank_id)
        REFERENCES sp_group_ranks (group_id, id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_supervisor_rule_granter FOREIGN KEY (granted_by_subject_id)
        REFERENCES sp_subjects (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_supervisor_rule_mode CHECK (rank_mode IN ('minimum', 'selected')),
    CONSTRAINT ck_sp_supervisor_rule_rank CHECK (
        (rank_mode = 'minimum' AND minimum_rank_id IS NOT NULL)
        OR (rank_mode = 'selected' AND minimum_rank_id IS NULL)
    ),
    CONSTRAINT ck_sp_supervisor_rule_review CHECK (can_review IN (0, 1)),
    CONSTRAINT ck_sp_supervisor_rule_dates CHECK (
        revoked_at_utc IS NULL OR revoked_at_utc >= granted_at_utc
    )
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Exact rank selection. An empty selection grants nobody access.
-- rank_mode is deliberate redundancy: the FK forbids selections on minimum-rank rules.
CREATE TABLE sp_zone_supervisor_ranks (
    zone_id BIGINT UNSIGNED NOT NULL,
    group_id BIGINT UNSIGNED NOT NULL,
    rank_id BIGINT UNSIGNED NOT NULL,
    rank_mode VARCHAR(8) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT 'selected',
    PRIMARY KEY (zone_id, group_id, rank_id),
    KEY ix_sp_supervisor_rank_rule (zone_id, group_id, rank_mode),
    KEY ix_sp_supervisor_rank_group (group_id, rank_id),
    CONSTRAINT fk_sp_supervisor_rank_rule FOREIGN KEY (zone_id, group_id, rank_mode)
        REFERENCES sp_zone_supervisor_rules (zone_id, group_id, rank_mode)
        ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_supervisor_rank_group FOREIGN KEY (group_id, rank_id)
        REFERENCES sp_group_ranks (group_id, id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_supervisor_rank_mode CHECK (rank_mode = 'selected')
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Reviews are append-only annotations. They do not overwrite the automatic verdict.
-- The service rechecks permission for this exact attempt before appending a review.
CREATE TABLE sp_attempt_reviews (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    attempt_id BIGINT UNSIGNED NOT NULL,
    reviewer_subject_id BIGINT UNSIGNED NOT NULL,
    assessment VARCHAR(20) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
    notes TEXT NOT NULL,
    reviewed_at_utc DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    PRIMARY KEY (id),
    KEY ix_sp_review_attempt (attempt_id, reviewed_at_utc, id),
    KEY ix_sp_review_author (reviewer_subject_id, reviewed_at_utc, id),
    CONSTRAINT fk_sp_review_attempt FOREIGN KEY (attempt_id)
        REFERENCES sp_attempts (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT fk_sp_review_author FOREIGN KEY (reviewer_subject_id)
        REFERENCES sp_subjects (id) ON DELETE RESTRICT ON UPDATE RESTRICT,
    CONSTRAINT ck_sp_review_assessment CHECK (assessment IN ('reviewed', 'needs_followup'))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
