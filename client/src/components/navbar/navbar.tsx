import { Card, CardHeader } from "~/components/card";
import { useNavigate } from "react-router-dom";
import styles from "./navbar.module.css";

const Navbar: React.FC = () => {
  const navigate = useNavigate();

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
