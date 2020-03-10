CREATE VIEW STATUS_FULL_INFO AS
    SELECT STATUS.id as id, STATUS.description as description, STATUS.created_at as posted_at,
         thumbs_up, username, screen_name, verified, img_src, reply_id
    FROM (
    STATUS LEFT JOIN PROFILE
    ON STATUS.profile_id = PROFILE.id
    )
    ORDER BY STATUS.created_at DESC;

CREATE VIEW STATS AS
     SELECT * 
     FROM
     (
          SELECT COUNT(*) AS profiles , MAX(followers_count) as max_followers, MIN(created_at) as oldest_acc, SUM(verified) as verified  
          FROM PROFILE
     ) 
     cross join 
     (
          SELECT COUNT(*) AS statuses, MAX(thumbs_up) as max_likes, MAX(LENGTH(description)) as longest_status 
          FROM STATUS
     );
     
