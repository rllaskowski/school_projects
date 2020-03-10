const oracledb = require('oracledb');

oracledb.outFormat = oracledb.OUT_FORMAT_OBJECT;
oracledb.autoCommit = true;


const config =  {
  user          : "rlaskowski",
  password      : "RLOracle1998",
  connectString : "localhost/orcl.home"
};

async function executeQuery(query, args) {
  let connection;

  try {
    connection = await oracledb.getConnection(config);

    const result = await connection.execute(
      query,
      args
    );
    return result;

  } catch (err) {
    console.log(err);
  } finally {
  }
}

module.exports = executeQuery;

