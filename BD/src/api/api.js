function getUser(username) {
    return fetch(`http://localhost:5000/api/getUser/${username}`).
    then((res)=>res.json()).then((res) => {console.log(res); return res.user});
}

function getUserByID(id) {
    return fetch(`http://localhost:5000/api/getUserByID/${id}`).
    then((res)=>res.json()).then(res => res.user);
}

function getAllUsers() {
    return fetch('http://localhost:5000/api/getUsers').
    then((res)=>res.json()); 
}

function getAllStatuses() {
    return fetch('http://localhost:5000/api/getStatuses').
    then((res)=>res.json());
}

function getUserStatuses(username) {
    return fetch(`http://localhost:5000/api/getUserStatuses/${username}`).
    then((res)=>res.json());
}

function getStatusByID(id) {
    return fetch(`http://localhost:5000/api/getStatus/${id}`).
    then((res)=>res.json()).then(res => res.status);
}

function getRepliesTo(id) {
    return fetch(`http://localhost:5000/api/getReplies/${id}`).
    then((res)=>res.json());
}

function getStats() {
    return fetch('http://localhost:5000/api/getStats').
    then((res)=>res.json());
}

export {
    getUser,
    getAllStatuses,
    getAllUsers,
    getUserStatuses,
    getStats,
    getUserByID,
    getStatusByID,
    getRepliesTo
}