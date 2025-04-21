import React from "react";
import styles from "./hide-icon.module.css";

interface HideIconProps {
  reverse?: boolean;
  className?: string;
  children?: string;
}

const HideIcon: React.FC<HideIconProps> = ({ reverse, className, children }) => {
  return (
    <div className={`${styles.hide_icon} ${className}`}>
      {children || (reverse ? ">" : "<")}
    </div>
  );
};

export default HideIcon;
