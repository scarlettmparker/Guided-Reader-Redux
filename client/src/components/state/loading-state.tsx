import React from "react";
import { LoaderCircle } from "~/components/lucide";

import styles from "./styles/loading-state.module.css";

interface LoadingStateProps {
  className?: string;
  children: React.ReactNode | string;
}

const LoadingState: React.FC<LoadingStateProps> = ({
  className = "",
  children,
}) => {
  return (
    <span className={`${styles.loading_state} ${className}`}>
      <LoaderCircle /> {children}
    </span>
  );
};

export default LoadingState;
