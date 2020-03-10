CREATE TABLE PROFILE (
    id NUMBER PRIMARY KEY NOT NULL,
    username VARCHAR(50) NOT NULL,
    screen_name VARCHAR(80) NOT NULL,
    img_src VARCHAR(200),
    background_src VARCHAR(200),
    favourites_statuses NUMBER NOT NULL,
    created_at DATE NOT NULL,
    followers_count NUMBER NOT NULL,
    verified NUMBER(1) NOT NULL,
    location VARCHAR(30)
);

CREATE TABLE STATUS (
    id NUMBER NOT NULL PRIMARY KEY,
    description VARCHAR(200) CHECK(LENGTH(description) <= 200),
    reply_id NUMBER REFERENCES STATUS(id),
    profile_id NUMBER NOT NULL,
    created_at DATE NOT NULL,
    thumbs_up NUMBER NOT NULL,
    FOREIGN KEY (profile_id) REFERENCES PROFILE(id)
);