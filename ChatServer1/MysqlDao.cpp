#include "MysqlDao.h"
#include "ConfigMgr.h"

MysqlDao::MysqlDao(){
	auto& cfg = ConfigMgr::Inst();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() {
	pool_->Close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd){
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return false;
		}
		// 准备调用存储过程
		std::unique_ptr < sql::PreparedStatement > stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// 设置输入参数
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, pwd);

		// 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

		  // 执行存储过程
		stmt->execute();
		// 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
	   // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
		if (res->next()) {
			int result = res->getInt("result");
			std::cout << "Result: " << result << std::endl;
			pool_->returnConnection(std::move(con));
			return result;
		}
		pool_->returnConnection(std::move(con));
		return -1;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
}

bool MysqlDao::CheckEmail(const std::string& name, const std::string& email) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			pool_->returnConnection(std::move(con));
			return false;
		}

		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt{ con->raw()->prepareStatement("SELECT email FROM user WHERE name = ?")};

		// 绑定参数
		pstmt->setString(1, name);

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		// 遍历结果集
		while (res->next()) {
			std::cout << "Check Email: " << res->getString("email") << std::endl;
			if (email != res->getString("email")) {
				pool_->returnConnection(std::move(con));
				return false;
			}
			pool_->returnConnection(std::move(con));
			return true;
		}
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::UpdatePwd(const std::string& name, const std::string& newpwd) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			pool_->returnConnection(std::move(con));
			return false;
		}

		// 准备查询语句
		std::unique_ptr<sql::PreparedStatement> pstmt{ con->raw()->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?")};

		// 绑定参数
		pstmt->setString(2, name);
		pstmt->setString(1, newpwd);

		// 执行更新
		int updateCount = pstmt->executeUpdate();

		std::cout << "Updated rows: " << updateCount << std::endl;
		pool_->returnConnection(std::move(con));
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo) {
	auto con = pool_->getConnection();
	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt{ con->raw()->prepareStatement("SELECT * FROM user WHERE name = ?") };
		pstmt->setString(1, name); // 将username替换为你要查询的用户名

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";
		// 遍历结果集
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			// 输出查询到的密码
			std::cout << "Password: " << origin_pwd << std::endl;
			break;
		}

		if (pwd != origin_pwd) {
			return false;
		}
		userInfo.name = name;
		userInfo.email = res->getString("email");
		userInfo.uid = res->getInt("uid");
		userInfo.pwd = origin_pwd;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

 bool MysqlDao::AddFriendApply(const int& from, const int& to)
 {
	 auto con = pool_->getConnection();
	 if (con == nullptr) {
		 return false;
	 }

	 Defer defer([this, &con]() {
		 pool_->returnConnection(std::move(con));
		 });

	 try {
		 
		 std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("INSERT INTO friend_apply (from_uid, to_uid) values (?,?) "
			 "ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = to_uid"));
		 pstmt->setInt(1, from);
		 pstmt->setInt(2, to);
		 
		 int rowAffected = pstmt->executeUpdate();
		 if (rowAffected < 0) {
			 return false;
		 }
		 return true;
	 }
	 catch (sql::SQLException& e) {
		 std::cerr << "SQLException: " << e.what();
		 std::cerr << " (MySQL error code: " << e.getErrorCode();
		 std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		 return false;
	 }


	 return true;
 }

 bool MysqlDao::AuthFriendApply(const int& from, const int& to) {
	 auto con = pool_->getConnection();
	 if (con == nullptr) {
		 return false;
	 }

	 Defer defer([this, &con]() {
		 pool_->returnConnection(std::move(con));
		 });

	 try {
		 
		 std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE friend_apply SET status = 1 "
			 "WHERE from_uid = ? AND to_uid = ?"));
		 
		 pstmt->setInt(1, to); // id
		 pstmt->setInt(2, from);
		 
		 int rowAffected = pstmt->executeUpdate();
		 if (rowAffected < 0) {
			 return false;
		 }
		 return true;
	 }
	 catch (sql::SQLException& e) {
		 std::cerr << "SQLException: " << e.what();
		 std::cerr << " (MySQL error code: " << e.getErrorCode();
		 std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		 return false;
	 }


	 return true;
 }

 bool MysqlDao::AddFriend(const int& from, const int& to, std::string back_name) {
	 auto con = pool_->getConnection();
	 if (con == nullptr) {
		 return false;
	 }

	 Defer defer([this, &con]() {
		 pool_->returnConnection(std::move(con));
		 });

	 try {

		 
		 con->_con->setAutoCommit(false);

		
		 std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			 "VALUES (?, ?, ?) "
		 ));
		 
		 pstmt->setInt(1, from); // id
		 pstmt->setInt(2, to);
		 pstmt->setString(3, back_name);
		
		 int rowAffected = pstmt->executeUpdate();
		 if (rowAffected < 0) {
			 con->_con->rollback();
			 return false;
		 }

		 
		 std::unique_ptr<sql::PreparedStatement> pstmt2(con->_con->prepareStatement("INSERT IGNORE INTO friend(self_id, friend_id, back) "
			 "VALUES (?, ?, ?) "
		 ));
		 
		 pstmt2->setInt(1, to); // from id
		 pstmt2->setInt(2, from);
		 pstmt2->setString(3, "");
		
		 int rowAffected2 = pstmt2->executeUpdate();
		 if (rowAffected2 < 0) {
			 con->_con->rollback();
			 return false;
		 }

		 
		 con->_con->commit();
		 std::cout << "addfriend insert friends success" << std::endl;

		 return true;
	 }
	 catch (sql::SQLException& e) {
		 
		 if (con) {
			 con->_con->rollback();
		 }
		 std::cerr << "SQLException: " << e.what();
		 std::cerr << " (MySQL error code: " << e.getErrorCode();
		 std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		 return false;
	 }


	 return true;
 }

 auto MysqlDao::GetUser(int uid) -> std::shared_ptr<UserInfo>
 {
	 auto con = pool_->getConnection();
	 if (con == nullptr) {
		 return nullptr;
	 }

	 Defer defer([this, &con]() {
		 pool_->returnConnection(std::move(con));
		 });

	 try {
		 
		 std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
		 pstmt->setInt(1, uid); 

		 
		 std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		 std::shared_ptr<UserInfo> user_ptr = nullptr;
		 
		 while (res->next()) {
			 user_ptr.reset(new UserInfo);
			 user_ptr->pwd = res->getString("pwd");
			 user_ptr->email = res->getString("email");
			 user_ptr->name = res->getString("name");
			 user_ptr->nick = res->getString("nick");
			 user_ptr->desc = res->getString("desc");
			 user_ptr->sex = res->getInt("sex");
			 user_ptr->icon = res->getString("icon");
			 user_ptr->uid = uid;
			 break;
		 }
		 return user_ptr;
	 }
	 catch (sql::SQLException& e) {
		 std::cerr << "SQLException: " << e.what();
		 std::cerr << " (MySQL error code: " << e.getErrorCode();
		 std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		 return nullptr;
	 }
 }

 std::shared_ptr<UserInfo> MysqlDao::GetUser(std::string name)
 {
	 auto con = pool_->getConnection();
	 if (con == nullptr) {
		 return nullptr;
	 }

	 Defer defer([this, &con]() {
		 pool_->returnConnection(std::move(con));
		 });

	 try {
		
		 std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE name = ?"));
		 pstmt->setString(1, name); 

		
		 std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		 std::shared_ptr<UserInfo> user_ptr = nullptr;
		 
		 while (res->next()) {
			 user_ptr.reset(new UserInfo);
			 user_ptr->pwd = res->getString("pwd");
			 user_ptr->email = res->getString("email");
			 user_ptr->name = res->getString("name");
			 user_ptr->nick = res->getString("nick");
			 user_ptr->desc = res->getString("desc");
			 user_ptr->sex = res->getInt("sex");
			 user_ptr->uid = res->getInt("uid");
			 break;
		 }
		 return user_ptr;
	 }
	 catch (sql::SQLException& e) {
		 std::cerr << "SQLException: " << e.what();
		 std::cerr << " (MySQL error code: " << e.getErrorCode();
		 std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		 return nullptr;
	 }
 }


 bool MysqlDao::GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit) {
	 auto con = pool_->getConnection();
	 if (con == nullptr) {
		 return false;
	 }

	 Defer defer([this, &con]() {
		 pool_->returnConnection(std::move(con));
		 });


	 try {
		 
		 std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select apply.from_uid, apply.status, user.name, "
			 "user.nick, user.sex from friend_apply as apply join user on apply.from_uid = user.uid where apply.to_uid = ? "
			 "and apply.id > ? order by apply.id ASC LIMIT ? "));

		 pstmt->setInt(1, touid); 
		 pstmt->setInt(2, begin); 
		 pstmt->setInt(3, limit); 
		
		 std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		 
		 while (res->next()) {
			 auto name = res->getString("name");
			 auto uid = res->getInt("from_uid");
			 auto status = res->getInt("status");
			 auto nick = res->getString("nick");
			 auto sex = res->getInt("sex");
			 auto apply_ptr = std::make_shared<ApplyInfo>(uid, name, "", "", nick, sex, status);
			 applyList.push_back(apply_ptr);
		 }
		 return true;
	 }
	 catch (sql::SQLException& e) {
		 std::cerr << "SQLException: " << e.what();
		 std::cerr << " (MySQL error code: " << e.getErrorCode();
		 std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		 return false;
	 }
 }

 bool MysqlDao::GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info_list) {

	 auto con = pool_->getConnection();
	 if (con == nullptr) {
		 return false;
	 }

	 Defer defer([this, &con]() {
		 pool_->returnConnection(std::move(con));
		 });


	 try {
		 
		 std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("select * from friend where self_id = ? "));

		 pstmt->setInt(1, self_id); 

		 
		 std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		
		 while (res->next()) {
			 auto friend_id = res->getInt("friend_id");
			 auto back = res->getString("back");

			 auto user_info = GetUser(friend_id);
			 if (user_info == nullptr) {
				 continue;
			 }

			 user_info->back = user_info->name;
			 user_info_list.push_back(user_info);
		 }
		 return true;
	 }
	 catch (sql::SQLException& e) {
		 std::cerr << "SQLException: " << e.what();
		 std::cerr << " (MySQL error code: " << e.getErrorCode();
		 std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		 return false;
	 }

	 return true;
 }