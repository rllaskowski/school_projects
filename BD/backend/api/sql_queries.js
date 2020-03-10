const insertUserSQL = `
INSERT INTO PROFILE (
    id,
    username,
    screen_name,
    img_src,
    favourites_statuses,
    background_src,
    created_at,
    followers_count,
    verified,
    location) 
    VALUES(
        :id,
        :username,
        :screenName,
        :imgSrc,
        :favouriteStatuses,
        :backgroundSrc,
        TO_DATE(:createdAt, 'MM/DD/YYYY'),
        :followersCount,
        :verified,
        :location
    )`;

const insertStatusSQL =  `
INSERT INTO STATUS (
    id,
    description,
    profile_id,
    created_at,
    thumbs_up,
    reply_id
    ) 
    VALUES(
        :id,
        :description,
        :profile_id,
        TO_DATE(:created_at, 'MM/DD/YYYY'),
        :thumbs_up,
        :reply_to
    )
`;

const selectAllUsersSQL = `
SELECT * FROM PROFILE
`;

const selectUserSQL = `
SELECT * FROM PROFILE
WHERE PROFILE.username = :username
`;

const selectUserByIdSQL = `
SELECT * FROM PROFILE
WHERE PROFILE.id = :id
`;

const selectAllStatusesSQL = `
SELECT * FROM STATUS_FULL_INFO
`;

const selectStatusSQL = `
SELECT * FROM  STATUS_FULL_INFO
WHERE id = :id
`;

const selectUserStatusesSQL = `
SELECT * FROM STATUS
WHERE profile_id = :id
`;


const selectRepliesSQL = `
SELECT * FROM  STATUS_FULL_INFO
WHERE reply_id = :id
`;

const selectStatsSQL = `
SELECT * FROM STATS
`;

module.exports = {
    insertStatusSQL,
    insertUserSQL,
    selectAllUsersSQL,
    selectUserSQL,
    selectUserByIdSQL,
    selectAllStatusesSQL,
    selectStatusSQL,
    selectRepliesSQL,
    selectUserStatusesSQL,
    selectStatsSQL
}
