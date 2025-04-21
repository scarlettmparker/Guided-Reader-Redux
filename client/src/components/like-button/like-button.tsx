import React, { useState } from "react";
import styles from "./like-button.module.css";

interface LikeButtonProps {
  className?: string;
  reverse?: boolean;
}

const LikeButton: React.FC<LikeButtonProps> = ({
  className = "",
  reverse = false,
}) => {
  const [active, setActive] = useState(false);

  const handleClick = () => {
    setActive(!active);
    // Perform API call
  };

  return (
    <img
      src={
        active
          ? "/assets/annotation/icons/voted.png"
          : "/assets/annotation/icons/unvoted.png"
      }
      alt="Like"
      className={`${styles.like_button} ${className}`}
      style={{ transform: reverse ? "rotate(180deg)" : "none" }}
      width="20"
      height="20"
      onClick={handleClick}
    />
  );
};

export default LikeButton;
