import React from "react";
import styles from "./hide-icon.module.css";

interface HideIconProps {
  reverse?: boolean;
  className?: string;
  children?: string;
  onClick?: () => void;
}

const HideIcon: React.FC<HideIconProps> = ({
  reverse,
  className,
  children,
  onClick,
}) => {
  return (
    <div className={`${styles.hide_icon} ${className}`} onClick={onClick}>
      {children || (reverse ? ">" : "<")}
    </div>
  );
};

export default HideIcon;
