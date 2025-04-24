import { Card, CardHeader } from "~/components/card";
import styles from "./navbar.module.css";

const Navbar: React.FC = () => {
  return (
    <Card className={styles.navbar}>
      <CardHeader>
        <span className={styles.navbar_header}>Login</span>
      </CardHeader>
    </Card>
  );
};

export default Navbar;
