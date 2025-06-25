import React from "react";
import styles from "./not-found.module.css";

const NotFound: React.FC = () => (
  <div className={styles.not_found}>
    <h1>404 - Page Not Found</h1>
    <p>The page you are looking for does not exist.</p>
  </div>
);

export default NotFound;
