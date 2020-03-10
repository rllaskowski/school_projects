const executeQuery = require('./dbQuery');
const queries = require('./sql_queries');

const tableToUser = user => {
    return {
        id: user.ID,
        username: user.USERNAME,
        screenName: user.SCREEN_NAME,
        description: user.DESCRIPTION,
        imgSrc: user.IMG_SRC,
        createdAt: user.CREATED_AT,
        followersCount: user.FOLLOWERS_COUNT,
        backgroundSrc: user.BACKGROUND_SRC,
        favouriteCount: user.FAVOURITES_STATUSES,
        verified: user.VERIFIED,
        location: user.LOCATION
    }
};

const viewToFullStatus = data => {
    return {
        user: {
            username: data.USERNAME,
            screenName: data.SCREEN_NAME,
            verified: data.VERIFIED,
            imgSrc: data.IMG_SRC
        },
        status: {
            id: data.ID,
            thumbsUp: data.THUMBS_UP,
            createdAt: data.POSTED_AT,
            description: data.DESCRIPTION
        }
    }
}

const tableToStatus = status => {
    return {
        id: status.ID,
        description: status.DESCRIPTION,
        reply_to: status.REPLY_ID,
        user_id: status.PROFILE_ID,
        createdAt: status.CREATED_AT,
        thumbsUp: status.THUMBS_UP
    }
}

Date.prototype.stringify = function() {
    var mm = this.getMonth() + 1; // getMonth() is zero-based
    var dd = this.getDate();

    return [
        (mm>9 ? '' : '0') + mm,
        (dd>9 ? '' : '0') + dd,
        this.getFullYear(),
    ].join('/');
};


const processFile = (fileString) => {
    const data = JSON.parse(fileString);

    for (status of  data.statuses) {
        const user = status.user;
        user.created_at = (new Date (Date.parse(user.created_at))).stringify();
        executeQuery(queries.insertUserSQL, [user.id, user.screen_name, user.name,
                 user.profile_image_url, user.favourites_count, 
                 user.profile_background_image_url, user.created_at, 
                 user.followers_count, user.verified? 1: 0, user.location]).then(res=>{
            console.log(res);
        });
        
        status.created_at = (new Date (Date.parse(status.created_at))).stringify();
        executeQuery(queries.insertStatusSQL, [status.id, status.text, user.id,
            status.created_at,
            status.favorite_count, status.in_reply_to_status_id]).then(res=>{
       console.log(res);
   });
    }
}

const viewToStats = (data) => {
    return {
        profiles: data.PROFILES,
        maxLikes: data.MAX_LIKES,
        maxFollowers: data.MAX_FOLLOWERS,
        statuses: data.STATUSES,
        verified: data.VERIFIED,
        longestStatus: data.LONGEST_STATUS,
        oldestAccount: data.OLDEST_ACC
    }
};


module.exports = {
    tableToUser,
    viewToFullStatus,
    tableToStatus,
    viewToStats,
    processFile
}