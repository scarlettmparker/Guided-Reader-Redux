import { Card, CardHeader } from "~/components/card";
import { useNavigate, useLocation } from "react-router-dom";
import styles from "./navbar.module.css";

const Navbar: React.FC = () => {
  const navigate = useNavigate();
  const location = useLocation();

  // List of routes where navbar should be hidden
  const hiddenRoutes = ["/login"];
  if (hiddenRoutes.includes(location.pathname)) {
    return null;
  }

  return (
    <Card className={styles.navbar}>
      <CardHeader className={styles.navbar_header}>
        <span
          className={styles.navbar_login}
          onClick={() => navigate("/login")}
        >
          Login
        </span>
      </CardHeader>
    </Card>
  );
};

export default Navbar;