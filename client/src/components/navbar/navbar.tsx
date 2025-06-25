import { Card, CardHeader } from "~/components/card";
import { useLocation } from "react-router-dom";
import { useUser } from "~/contexts/user-context";
import ProfilePicture from "../profile-picture";
import styles from "./navbar.module.css";
import { UserController } from "~/utils/api";
import { useNavigate } from "react-router-dom";

const Navbar: React.FC = () => {
  const location = useLocation();
  const navigate = useNavigate();
  const { user } = useUser();

  // List of routes where navbar should be hidden
  const hiddenRoutes = ["/login"];
  if (hiddenRoutes.includes(location.pathname)) {
    return null;
  }

  return (
    <Card className={styles.navbar}>
      <CardHeader className={styles.navbar_header}>
        {user && (
          <div className={styles.navbar_user}>
            <ProfilePicture avatar={user.avatar} discord_id={user.discord_id} />
            <span className={styles.navbar_username}>{user.username}</span>
          </div>
        )}
        <span
          className={styles.navbar_login}
          onClick={() => {
            user ? UserController.logout(user.id) : navigate("/login");
          }}
        >
          {user ? "Logout" : "Login"}
        </span>
      </CardHeader>
    </Card>
  );
};

export default Navbar;
