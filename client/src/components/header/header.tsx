import React from "react";
import styles from "./header.module.css";

interface HeaderProps {
  children?: React.ReactNode;
}

const Header: React.FC<HeaderProps> = ({ children }) => {
  return <div className={styles.header}>{children ? children : ""}</div>;
};

export default Header;
